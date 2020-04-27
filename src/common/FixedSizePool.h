// Copyright (C) 2018-2020 European Spallation Source, ERIC. See LICENSE file
#pragma once

#include <common/Assert.h>
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

#define FIXED_SIZE_POOL_DISABLE_ALL_CHECKS 0

constexpr size_t NextPowerOfTwo(size_t n) {
  n--;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  n |= n >> 32;
  n++;
  return n;
}

/// \class FixedSizePoolParams
/// \brief Contains the compile-time constants and parameters for \class
///        FixedSizePool. This makes the template implementation less cluttery.
template <size_t SlotBytes_, size_t NumSlots_,
          size_t SlotAlignment_ = SlotBytes_, size_t StartAlignment_ = 16,
          bool Validate_ = true, bool UseAsserts_ = true>
struct FixedSizePoolParams {
  enum : size_t {
    SlotBytes = NextPowerOfTwo(std::max(SlotBytes_, SlotAlignment_)),
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

  enum : unsigned char { MemDeletedPattern = 0xED, MemAllocatedPattern = 0xCD };

  static_assert((SlotBytes & (SlotBytes - 1)) == 0,
                "SlotBytes must be power-of-two. Consider arounding up to "
                "nearest pwer-of-two");

  uint32_t NumSlotsUsed;
  uint32_t FreeSlotStack[NumSlots];
  alignas(StartAlignment) unsigned char PoolBytes[SlotBytes * NumSlots];

  FixedSizePool();
  ~FixedSizePool();

  void *AllocateSlot();
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
  }

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
void *FixedSizePool<FixedSizePoolParamsT>::AllocateSlot() {
  if (NumSlotsUsed == NumSlots) {
    return nullptr;
  }

  size_t index = FreeSlotStack[NumSlotsUsed];
  PoolAssertMsg(UseAsserts, index < NumSlots, "Expect capacity");

  NumSlotsUsed++;
  unsigned char *p = PoolBytes + (index * SlotBytes);
  PoolAssertMsg(UseAsserts, p + SlotBytes <= PoolBytes + sizeof(PoolBytes),
                "Don't go past end of capacity");

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
  size_t index = ((unsigned char *)p - PoolBytes) / SlotBytes;
  PoolAssertMsg(UseAsserts, index < NumSlots, "Dealloc pointer is not from pool");

  PoolAssertMsg(UseAsserts, NumSlotsUsed > 0, "Pool is already empty");
  --NumSlotsUsed;

  FreeSlotStack[NumSlotsUsed] = index;

  if (Validate) {
    memset(p, MemDeletedPattern, SlotBytes);
  }
}

template <typename FixedSizePoolParamsT>
bool FixedSizePool<FixedSizePoolParamsT>::Contains(void *p) {
  return (unsigned char *)p >= PoolBytes &&
         (unsigned char *)p < PoolBytes + sizeof(PoolBytes);
}
