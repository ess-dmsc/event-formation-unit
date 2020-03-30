#pragma once

#include <common/Assert.h>
#include <common/FixedSizePool.h>
#include <common/Trace.h>

#include <cstdint>

template <typename FixedPoolConfigT> struct PoolAllocator {
  using T = typename FixedPoolConfigT::T;
  typedef T value_type;
  typedef T &reference;
  typedef const T &const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef T *pointer;
  typedef const T *const_pointer;

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

  XTRACE(MAIN, CRI,
         "No pool for alloc: PoolAllocator %u objs, %u bytes, %u "
         "maxBytes",
         n, sizeof(T) * n, m_Pool.kSlotBytes);
  RelAssertMsg(0, "No pool for alloc");
  // throw std::bad_alloc();
  return NULL;
}

template <typename FixedPoolConfigT>
void PoolAllocator<FixedPoolConfigT>::deallocate(T *p, std::size_t) noexcept {
  m_Pool.DeallocateSlot(p);
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