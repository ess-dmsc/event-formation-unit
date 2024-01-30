// Copyright (C) 2020-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Template based, fixed pool size allocator (speed optimisation)
//===----------------------------------------------------------------------===//

#pragma once

#include <common/BitMath.h>
#include <common/debug/Assert.h>
#include <common/debug/Expect.h>
#include <common/debug/Trace.h>

#include <algorithm>
#include <bitset>
#include <cstdint>

#define PoolAssertMsg(kEnable, ...)                                            \
  do {                                                                         \
    if (kEnable)                                                               \
      RelAssertMsg(__VA_ARGS__);                                               \
  } while (0)

// undef TRC_LEVEL
// define TRC_LEVEL TRC_L_DEB

template <typename T> inline constexpr T EfuMax(T a, T b) {
  return a > b ? a : b;
}
template <typename T> inline constexpr T EfuMin(T a, T b) {
  return a < b ? a : b;
}

#define FIXED_SIZE_POOL_DISABLE_ALL_CHECKS 0

/// \class FixedSizePoolParams
/// \brief Contains the compile-time constants and parameters for \class
///        FixedSizePool. This makes the template implementation less cluttery.
template <size_t SlotBytes_, size_t NumSlots_,
          size_t SlotAlignment_ = SlotBytes_, size_t StartAlignment_ = 16,
          bool Validate_ = true, bool UseAsserts_ = true>
struct FixedSizePoolParams {
  enum : size_t {
    SlotBytes = BitMath::NextPowerOfTwo(std::max(SlotBytes_, SlotAlignment_)),
    NumSlots = NumSlots_,
    SlotAlignment = SlotAlignment_,
    StartAlignment = std::max(SlotAlignment_, StartAlignment_),
#if FIXED_SIZE_POOL_DISABLE_ALL_CHECKS
    Validate = false,
    UseAsserts = false
#else
    Validate = Validate_,
    UseAsserts = UseAsserts_
#endif
  };
};

/// \class FixedSizePool
/// \brief This keeps a stack of available indices (FreeSlotStack) into block of
///        memory (PoolBytes), which is partitioned into NumSlots slots of
///        SlotBytes. Allocation and deallocation is respectivly done by popping
///        and pushing from the stack.
/// The validation mode (Validate) fills the slots with a pattern
///        (MemDeletedPattern, MemAllocatedPattern) to make it easier to detect
///        use-after-free. It also checks on destruction that all indices in the
///        stack are unique, meaning no double-free.
template <typename FixedSizePoolParamsT> struct FixedSizePool {
  enum : size_t {
    SlotBytes = FixedSizePoolParamsT::SlotBytes,
    NumSlots = FixedSizePoolParamsT::NumSlots,
    SlotAlignment = FixedSizePoolParamsT::SlotAlignment,
    StartAlignment = FixedSizePoolParamsT::StartAlignment,
    Validate = FixedSizePoolParamsT::Validate,
    UseAsserts = FixedSizePoolParamsT::UseAsserts,
  };

  /// \note Doing stats wastes from performance due to cache misses.
  ///       Data needs to be int64 as required by common::Statstics.
  struct MemStats {
    int64_t AllocCount;
    int64_t AllocBytes;
    int64_t DeallocCount;
    int64_t DeallocBytes;
    int64_t MallocFallbackCount; // tracked by outside allocator
  };

  enum : unsigned char { MemDeletedPattern = 0xED, MemAllocatedPattern = 0xCD };

  static_assert((SlotBytes & (SlotBytes - 1)) == 0,
                "SlotBytes must be power-of-two. Consider arounding up to "
                "nearest pwer-of-two");

  uint32_t NumSlotsUsed;
  MemStats Stats;
  uint32_t FreeSlotStack[NumSlots]; // no order
  uint32_t SlotAllocSize[NumSlots]; // indexed by Slot index
  alignas(StartAlignment) unsigned char PoolBytes[SlotBytes * NumSlots];

  FixedSizePool();

  void *AllocateSlot(size_t byteCount = SlotBytes);
  void DeallocateSlot(void *p);
  bool Contains(void *p);
  /// \return null on no error, else returns error description
  const char *ValidateEmptyStateAndReturnError();
};

template <typename FixedSizePoolParamsT>
FixedSizePool<FixedSizePoolParamsT>::FixedSizePool() {
  XTRACE(MAIN, DEB,
         "FixedSizePool: SlotBytes %u, NumSlots %u, SlotAlignment %u, "
         "StartAlignment %u, Validate %u, TotalBytes %u",
         (uint32_t)SlotBytes, (uint32_t)NumSlots, (uint32_t)SlotAlignment,
         (uint32_t)StartAlignment, Validate ? 1 : 0, (uint32_t)sizeof(*this));

  NumSlotsUsed = 0;
  for (size_t i = 0; i < NumSlots; ++i) {
    FreeSlotStack[i] = i;
    SlotAllocSize[i] = 0;
  }

  Stats.AllocCount = 0;
  Stats.AllocBytes = 0;
  Stats.DeallocCount = 0;
  Stats.DeallocBytes = 0;
  Stats.MallocFallbackCount = 0;

  if (Validate) {
    memset(PoolBytes, MemDeletedPattern, sizeof(PoolBytes));
  }
}

template <typename FixedSizePoolParamsT>
void *FixedSizePool<FixedSizePoolParamsT>::AllocateSlot(size_t byteCount) {
  if (UNLIKELY(NumSlotsUsed == NumSlots)) {
    return nullptr;
  }

  size_t slotIndex = FreeSlotStack[NumSlotsUsed];
  PoolAssertMsg(UseAsserts, slotIndex < NumSlots, "Expect capacity");

  PoolAssertMsg(UseAsserts, SlotAllocSize[slotIndex] == 0,
                "Expect slot to be empty");
  PoolAssertMsg(UseAsserts, byteCount <= 0xFFFFFFFFull,
                "Not expecting >32 bit alloc");

  NumSlotsUsed++;
  unsigned char *p = PoolBytes + (slotIndex * SlotBytes);
  PoolAssertMsg(UseAsserts, p + SlotBytes <= PoolBytes + sizeof(PoolBytes),
                "Don't go past end of capacity");

  Stats.AllocCount++;
  Stats.AllocBytes += (int64_t)byteCount;
  SlotAllocSize[slotIndex] = (uint32_t)byteCount;

  if (Validate) {
    for (size_t i = 0; i < SlotBytes; ++i) {
      PoolAssertMsg(UseAsserts, p[i] == MemDeletedPattern,
                    "Unused memory must have deletion pattern");
    }
    memset(p, MemAllocatedPattern, SlotBytes);
  }

  return static_cast<void *>(p);
}

template <typename FixedSizePoolParamsT>
void FixedSizePool<FixedSizePoolParamsT>::DeallocateSlot(void *p) {
  size_t slotIndex = ((unsigned char *)p - PoolBytes) / SlotBytes;
  PoolAssertMsg(UseAsserts, slotIndex < NumSlots,
                "Dealloc pointer is not from pool");

  PoolAssertMsg(UseAsserts, NumSlotsUsed > 0, "Pool is already empty");
  --NumSlotsUsed;

  PoolAssertMsg(UseAsserts, SlotAllocSize[slotIndex] != 0,
                "Excepting dealloc slot to be in use.");

  Stats.DeallocCount++;
  Stats.DeallocBytes += (int64_t)SlotAllocSize[slotIndex];
  SlotAllocSize[slotIndex] = 0;

  FreeSlotStack[NumSlotsUsed] = slotIndex;

  if (Validate) {
    memset(p, MemDeletedPattern, SlotBytes);
  }
}

template <typename FixedSizePoolParamsT>
bool FixedSizePool<FixedSizePoolParamsT>::Contains(void *p) {
  return (unsigned char *)p >= PoolBytes &&
         (unsigned char *)p < PoolBytes + sizeof(PoolBytes);
}

template <typename FixedSizePoolParamsT>
const char *
FixedSizePool<FixedSizePoolParamsT>::ValidateEmptyStateAndReturnError() {
  if (NumSlotsUsed != 0) {
    return "All slots in pool must be empty";
  }
  if (Validate) {
    // test FreeSlotStack indices are unique
    {
      std::bitset<NumSlots> &foundSlots = *new std::bitset<NumSlots>();
      for (size_t i = 0; i < NumSlots; ++i) {
        if (SlotAllocSize[i] != 0) {
          delete &foundSlots;
          return "Slot not properly deallocated";
        }
        uint32_t curSlot = FreeSlotStack[i];
        if (foundSlots[curSlot]) {
          delete &foundSlots;
          return "Free slots must be unique. Could mean double delete";
        }
        foundSlots[curSlot] = true;
      }
      delete &foundSlots;
    }
    // test for deletion reference pattern
    for (size_t i = 0; i < sizeof(PoolBytes); ++i) {
      if (PoolBytes[i] != MemDeletedPattern) {
        return "Deleted memory must have reference pattern";
      }
    }
  }
  return nullptr;
}
