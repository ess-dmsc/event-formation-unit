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

template <size_t kSlotBytes_, size_t kNumSlots_,
          size_t kSlotAlignment_ = kSlotBytes_, size_t kStartAlignment_ = 16,
          bool kValidate_ = true, bool kUseAsserts_ = true>
struct FixedSizePoolParams {
  enum : size_t {
    kSlotBytes = std::max(kSlotBytes_, kSlotAlignment_),
    kNumSlots = kNumSlots_,
    kSlotAlignment = kSlotAlignment_,
    kStartAlignment = std::max(kSlotAlignment_, kStartAlignment_),
    kValidate = kValidate_,
    kUseAsserts = kUseAsserts_
  };
};

template <typename FixedSizePoolParamsT> struct FixedSizePool {
  enum : size_t {
    kSlotBytes = FixedSizePoolParamsT::kSlotBytes,
    kNumSlots = FixedSizePoolParamsT::kNumSlots,
    kSlotAlignment = FixedSizePoolParamsT::kSlotAlignment,
    kStartAlignment = FixedSizePoolParamsT::kStartAlignment,
#if FIXED_SIZE_POOL_DISABLE_ALL_CHECKS
    kValidate = false,
    kUseAsserts = false
#else
    kValidate = FixedSizePoolParamsT::kValidate,
    kUseAsserts = FixedSizePoolParamsT::kUseAsserts,
#endif
  };
  enum : unsigned char { kMemDeletedPat = 0xED, kMemAllocatedPat = 0xCD };

  uint32_t m_NumSlotsUsed;
  uint32_t m_NextFreeSlot[kNumSlots];
  alignas(kStartAlignment) unsigned char m_PoolBytes[kSlotBytes * kNumSlots];

  FixedSizePool();
  ~FixedSizePool();

  void *AllocateSlot();
  void DeallocateSlot(void *p);
  bool Contains(void *p);
};

template <typename FixedSizePoolParamsT>
FixedSizePool<FixedSizePoolParamsT>::FixedSizePool() {
  XTRACE(MAIN, DEB,
         "FixedSizePool: kSlotBytes %u, kNumSlots %u, kSlotAlignment %u, "
         "kStartAlignment %u, kValidate %u, TotalBytes %u",
         (uint32_t)kSlotBytes, (uint32_t)kNumSlots, (uint32_t)kSlotAlignment,
         (uint32_t)kStartAlignment, kValidate ? 1 : 0, (uint32_t)sizeof(*this));

  m_NumSlotsUsed = 0;
  for (size_t i = 0; i < kNumSlots; ++i) {
    m_NextFreeSlot[i] = i;
  }

  if (kValidate) {
    memset(m_PoolBytes, kMemDeletedPat, sizeof(m_PoolBytes));
  }
}

template <typename FixedSizePoolParamsT>
FixedSizePool<FixedSizePoolParamsT>::~FixedSizePool() {
  PoolAssertMsg(kUseAsserts, m_NumSlotsUsed == 0,
                "All slots in pool must be empty");

  if (kValidate) {
    // test m_NextFreeSlot indices are unique
    {
      std::bitset<kNumSlots> &foundSlots = *new std::bitset<kNumSlots>();
      for (size_t i = 0; i < kNumSlots; ++i) {
        uint32_t curSlot = m_NextFreeSlot[i];
        PoolAssertMsg(kUseAsserts, !foundSlots[curSlot],
                      "Free slots must be unique. Could mean double delete");
        foundSlots[curSlot] = true;
      }
      delete &foundSlots;
    }
    // test for deletion reference pattern
    for (size_t i = 0; i < sizeof(m_PoolBytes); ++i) {
      PoolAssertMsg(kUseAsserts, m_PoolBytes[i] == kMemDeletedPat,
                    "Deleted memory must have reference pattern");
    }
  }
}

template <typename FixedSizePoolParamsT>
void *FixedSizePool<FixedSizePoolParamsT>::AllocateSlot() {
  PoolAssertMsg(kUseAsserts, m_NumSlotsUsed < kNumSlots,
                "Implement fallover to malloc()");
  size_t index = m_NextFreeSlot[m_NumSlotsUsed];
  PoolAssertMsg(kUseAsserts, index < kNumSlots, "Expect capacity");

  m_NumSlotsUsed++;
  unsigned char *p = m_PoolBytes + (index * kSlotBytes);
  PoolAssertMsg(kUseAsserts,
                p + kSlotBytes <= m_PoolBytes + sizeof(m_PoolBytes),
                "Don't go past end of capacity");

  if (kValidate) {
    for (size_t i = 0; i < kSlotBytes; ++i) {
      PoolAssertMsg(kUseAsserts, p[i] == kMemDeletedPat,
                    "Unused memory must have deletion pattern");
    }
    memset(p, kMemAllocatedPat, kSlotBytes);
  }

  return static_cast<void *>(p);
}

template <typename FixedSizePoolParamsT>
void FixedSizePool<FixedSizePoolParamsT>::DeallocateSlot(void *p) {
  size_t index = ((unsigned char *)p - m_PoolBytes) / kSlotBytes;
  PoolAssertMsg(kUseAsserts, index < kNumSlots, "Implement fallover to free()");

  PoolAssertMsg(kUseAsserts, m_NumSlotsUsed > 0, "Pool must have content");
  --m_NumSlotsUsed;

  m_NextFreeSlot[m_NumSlotsUsed] = index;

  if (kValidate) {
    memset(p, kMemDeletedPat, kSlotBytes);
  }
}

template <typename FixedSizePoolParamsT>
bool FixedSizePool<FixedSizePoolParamsT>::Contains(void *p) {
  return (unsigned char *)p >= m_PoolBytes &&
         (unsigned char *)p < m_PoolBytes + sizeof(m_PoolBytes);
}
