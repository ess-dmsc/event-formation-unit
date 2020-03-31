#pragma once

#include <common/Assert.h>
#include <common/FixedSizePool.h>
#include <common/Trace.h>

#include <cstdint>

template <typename FixedPoolConfigT> struct PoolAllocator {
  using T = typename FixedPoolConfigT::T;
  using value_type = T;
  using PoolType = typename FixedPoolConfigT::PoolType;

  PoolType &m_Pool;

  PoolAllocator(const PoolAllocator &) = default;
  PoolAllocator &operator=(const PoolAllocator &) = delete;

  PoolAllocator(PoolType &pool) noexcept : m_Pool(pool) {}

  template <typename U> struct rebind {
    using other =
        PoolAllocator<FixedPoolConfig<U, FixedPoolConfigT::kTotalBytes,
                                      FixedPoolConfigT::kObjectsPerSlot>>;
  };

  T *allocate(std::size_t n);
  void deallocate(T *p, std::size_t) noexcept;
};

template <typename FixedPoolConfigT>
typename FixedPoolConfigT::T *
PoolAllocator<FixedPoolConfigT>::allocate(std::size_t n) {
  if (sizeof(T) * n <= m_Pool.kSlotBytes)
    return (T *)m_Pool.AllocateSlot();

  T* heap = new T[n];
  //XTRACE(MAIN, CRI, "PoolAlloc fallover: %u objs, %u bytes", n, sizeof(T) * n);
  return heap;
}

template <typename FixedPoolConfigT>
void PoolAllocator<FixedPoolConfigT>::deallocate(T *p, std::size_t) noexcept {
  if (m_Pool.Contains(p)) {
    m_Pool.DeallocateSlot(p);
  } else {
    delete[] p;
  }
}

template <typename FixedPoolConfigT, typename FixedPoolConfigU>
bool operator==(const PoolAllocator<FixedPoolConfigT> &,
                const PoolAllocator<FixedPoolConfigU> &) {
  return true;
}

template <typename FixedPoolConfigT, typename FixedPoolConfigU>
bool operator!=(const PoolAllocator<FixedPoolConfigT> &,
                const PoolAllocator<FixedPoolConfigU> &) {
  return false;
}
