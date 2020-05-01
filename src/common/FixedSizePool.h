// Copyright (C) 2018-2020 European Spallation Source, ERIC. See LICENSE file
#pragma once

#include <common/Assert.h>
#include <common/BitMath.h>
#include <common/Trace.h>

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
    Validate = Validate_,
    UseAsserts = UseAsserts_
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
#if FIXED_SIZE_POOL_DISABLE_ALL_CHECKS
    Validate = false,
    UseAsserts = false
#else
    Validate = FixedSizePoolParamsT::Validate,
    UseAsserts = FixedSizePoolParamsT::UseAsserts,
#endif
  };

  /// \note Doing stats wastes from performance due to cache misses.
  ///       Data needs to be int64 as required by common::Statstics.
  struct MemStats {
    int64_t TotalBytes;
    int64_t LargestByteAlloc;
    int64_t AllocCount;
    int64_t MaxAllocCount;
    int64_t AccumAllocCount;
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
  ~FixedSizePool();

  void *AllocateSlot(size_t byteCount = SlotBytes);
  void DeallocateSlot(void *p);
  bool Contains(void *p);
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

  Stats.TotalBytes = 0;
  Stats.LargestByteAlloc = 0;
  Stats.AllocCount = 0;
  Stats.MaxAllocCount = 0;
  Stats.AccumAllocCount = 0;

  if (Validate) {
    memset(PoolBytes, MemDeletedPattern, sizeof(PoolBytes));
  }
}

template <typename FixedSizePoolParamsT>
FixedSizePool<FixedSizePoolParamsT>::~FixedSizePool() {
  PoolAssertMsg(UseAsserts, NumSlotsUsed == 0,
                "All slots in pool must be empty");

  if (Validate) {
    // test FreeSlotStack indices are unique
    {
      std::bitset<NumSlots> &foundSlots = *new std::bitset<NumSlots>();
      for (size_t i = 0; i < NumSlots; ++i) {
        PoolAssertMsg(UseAsserts, SlotAllocSize[i] == 0,
                      "Slot not properly deallocated");

        uint32_t curSlot = FreeSlotStack[i];
        PoolAssertMsg(UseAsserts, !foundSlots[curSlot],
                      "Free slots must be unique. Could mean double delete");
        foundSlots[curSlot] = true;
      }
      delete &foundSlots;
    }
    // test for deletion reference pattern
    for (size_t i = 0; i < sizeof(PoolBytes); ++i) {
      PoolAssertMsg(UseAsserts, PoolBytes[i] == MemDeletedPattern,
                    "Deleted memory must have reference pattern");
    }
  }
}

template <typename FixedSizePoolParamsT>
void *FixedSizePool<FixedSizePoolParamsT>::AllocateSlot(size_t byteCount) {
  if (__builtin_expect(NumSlotsUsed == NumSlots, 0)) {
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

  Stats.TotalBytes += (int64_t)byteCount;
  Stats.LargestByteAlloc = EfuMax(Stats.LargestByteAlloc, (int64_t)byteCount);
  Stats.AllocCount++;
  Stats.MaxAllocCount = EfuMax(Stats.MaxAllocCount, Stats.AllocCount);
  Stats.AccumAllocCount++;
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

  Stats.TotalBytes -= (int64_t)SlotAllocSize[slotIndex];
  Stats.AllocCount--;
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
