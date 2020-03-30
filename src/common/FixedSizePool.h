#pragma once

#include <common/Assert.h>
#include <common/Trace.h>

#include <algorithm>
#include <cstdint>

#if 1 /* option for speed tests */
#define PoolAssertMsg(...) RelAssertMsg(__VA_ARGS__)
#else
#define PoolAssertMsg(...)
#endif

// undef TRC_LEVEL
// define TRC_LEVEL TRC_L_DEB

template <size_t kSlotBytes_, size_t kNumSlots,
          size_t kSlotAlignment = kSlotBytes_, size_t kStartAlignment_ = 16,
          bool kValidate = true>
class FixedSizePool {
public:
  static const size_t kSlotBytes = std::max(kSlotBytes_, kSlotAlignment);
  static const size_t kStartAlignment =
      std::max(kSlotAlignment, kStartAlignment_);

  static const unsigned char kMemDeletedPat = 0xED;
  static const unsigned char kMemAllocatedPat = 0xCD;

  uint32_t m_NumSlotsUsed;
  uint32_t m_NextFreeSlot[kNumSlots];
  alignas(kStartAlignment) unsigned char m_PoolBytes[kSlotBytes * kNumSlots];

  FixedSizePool();
  ~FixedSizePool();

  void *AllocateSlot();
  void DeallocateSlot(void *p);
};

template <class T_, size_t kTotalBytes_, size_t kObjectsPerSlot_>
struct FixedPoolConfig {
  using T = T_;
  enum : size_t {
    kTotalBytes = kTotalBytes_,
    kObjectsPerSlot = kObjectsPerSlot_,
    kSlotBytes = sizeof(T) * kObjectsPerSlot,
    kNumSlots = kTotalBytes / kSlotBytes
  };

  static_assert(kTotalBytes >= kSlotBytes,
                "PoolAllocator must have enough bytes for one slot. Is "
                "kObjectsPerSlot sensible?");

  using PoolType = FixedSizePool<kSlotBytes, kNumSlots, alignof(T), 16, true>;
};

template <size_t kSlotBytes, size_t kNumSlots, size_t kSlotAlignment,
          size_t kStartAlignment, bool kValidate>
FixedSizePool<kSlotBytes, kNumSlots, kSlotAlignment, kStartAlignment,
              kValidate>::FixedSizePool() {
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

template <size_t kSlotBytes, size_t kNumSlots, size_t kSlotAlignment,
          size_t kStartAlignment, bool kValidate>
FixedSizePool<kSlotBytes, kNumSlots, kSlotAlignment, kStartAlignment,
              kValidate>::~FixedSizePool() {
  PoolAssertMsg(m_NumSlotsUsed == 0, "All slots in pool must be empty");

  if (kValidate) {
    // test m_NextFreeSlot indices are unique
    for (size_t testIndex = 0; testIndex < kNumSlots; ++testIndex) {
      __attribute__((unused)) bool testIndexFound = false;
      for (size_t i = 0; i < kNumSlots; ++i) {
        if (m_NextFreeSlot[i] == testIndex) {
          PoolAssertMsg(!testIndexFound,
                        "Free slots must be unique. Could mean double delete");
          testIndexFound = true;
        }
      }
      PoolAssertMsg(testIndexFound, "All slots must be used");
    }

    // test for deletion reference pattern
    for (size_t i = 0; i < sizeof(m_PoolBytes); ++i) {
      PoolAssertMsg(m_PoolBytes[i] == kMemDeletedPat,
                    "Deleted memory must have reference pattern");
    }
  }
}

template <size_t kSlotBytes, size_t kNumSlots, size_t kSlotAlignment,
          size_t kStartAlignment, bool kValidate>
void *FixedSizePool<kSlotBytes, kNumSlots, kSlotAlignment, kStartAlignment,
                    kValidate>::AllocateSlot() {
  PoolAssertMsg(m_NumSlotsUsed < kNumSlots, "Implement fallover to malloc()");
  size_t index = m_NextFreeSlot[m_NumSlotsUsed];
  PoolAssertMsg(index < kNumSlots, "Expect capacity");

  m_NumSlotsUsed++;
  unsigned char *p = m_PoolBytes + (index * kSlotBytes);
  PoolAssertMsg(p + kSlotBytes <= m_PoolBytes + sizeof(m_PoolBytes),
                "Don't go past end of capacity");

  if (kValidate) {
    for (size_t i = 0; i < kSlotBytes; ++i) {
      PoolAssertMsg(p[i] == kMemDeletedPat,
                    "Unused memory must have deletion pattern");
    }
    memset(p, kMemAllocatedPat, kSlotBytes);
  }

  return static_cast<void *>(p);
}

template <size_t kSlotBytes, size_t kNumSlots, size_t kSlotAlignment,
          size_t kStartAlignment, bool kValidate>
void FixedSizePool<kSlotBytes, kNumSlots, kSlotAlignment, kStartAlignment,
                   kValidate>::DeallocateSlot(void *p) {
  size_t index = ((unsigned char *)p - m_PoolBytes) / kSlotBytes;
  PoolAssertMsg(index < kNumSlots, "Implement fallover to free()");

  PoolAssertMsg(m_NumSlotsUsed > 0, "Pool must have content");
  --m_NumSlotsUsed;

  m_NextFreeSlot[m_NumSlotsUsed] = index;

  if (kValidate) {
    memset(p, kMemDeletedPat, kSlotBytes);
  }
}
