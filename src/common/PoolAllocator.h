#pragma once

#include <common/Assert.h>
#include <common/FixedSizePool.h>
#include <common/Trace.h>

template <class T, size_t kTotalBytes, size_t kObjectsPerSlot>
struct PoolAllocator {
  typedef T value_type;

  enum : size_t { kSlotBytes = sizeof(T) * kObjectsPerSlot };
  enum : size_t { kNumSlots = kTotalBytes / kSlotBytes };

  static_assert(kTotalBytes >= kSlotBytes,
                "PoolAllocator must have enough bytes for one slot. Is "
                "kObjectsPerSlot sensible?");

  FixedSizePool<kSlotBytes, kNumSlots, alignof(T), 16, true> m_Pool;

  PoolAllocator() {
    XTRACE(MAIN, DEB, "PoolAllocator: kTotalBytes %u", (uint32_t)kTotalBytes);
  }

  template <class U, size_t U_sz, size_t U_num>
  constexpr PoolAllocator(const PoolAllocator<U, U_sz, U_num> &) noexcept {}

  template <typename U> struct rebind {
    using other = PoolAllocator<U, kTotalBytes, kObjectsPerSlot>;
  };

  T *allocate(std::size_t n);
  void deallocate(T *p, std::size_t) noexcept;
};

template <class T, size_t kTotalBytes, size_t kObjectsPerSlot>
T *PoolAllocator<T, kTotalBytes, kObjectsPerSlot>::allocate(std::size_t n) {
  if (sizeof(T) * n <= m_Pool.kSlotBytes)
    return (T *)m_Pool.AllocateSlot();

  XTRACE(MAIN, CRI,
         "No pool for alloc: PoolAllocator %u objs, %u bytes, %u "
         "maxBytes",
         n, sizeof(T) * n, m_Pool.kSlotBytes);
  RelAssertMsg(0, "No pool for alloc");
  // throw std::bad_alloc();
  return NULL;
}

template <class T, size_t kTotalBytes, size_t kObjectsPerSlot>
void PoolAllocator<T, kTotalBytes, kObjectsPerSlot>::deallocate(
    T *p, std::size_t) noexcept {
  m_Pool.DeallocateSlot(p);
}

template <class T, size_t T_sz, size_t T_num, class U, size_t U_sz,
          size_t U_num>
bool operator==(const PoolAllocator<T, T_sz, T_num> &,
                const PoolAllocator<U, U_sz, U_num> &) {
  return true;
}
template <class T, size_t T_sz, size_t T_num, class U, size_t U_sz,
          size_t U_num>
bool operator!=(const PoolAllocator<T, T_sz, T_num> &,
                const PoolAllocator<U, U_sz, U_num> &) {
  return false;
}