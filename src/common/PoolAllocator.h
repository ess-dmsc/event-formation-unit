#pragma once

#include <common/Assert.h>
#include <common/FixedSizePool.h>
#include <common/Trace.h>

#include <cstdint>

template <class T_, size_t kTotalBytes_, size_t kObjectsPerSlot_,
          bool kValidate_ = true>
struct PoolAllocatorConfig {
  using T = T_;
  enum : size_t {
    kTotalBytes = kTotalBytes_,
    kObjectsPerSlot = kObjectsPerSlot_,
    kSlotBytes = sizeof(T) * kObjectsPerSlot,
    kNumSlots = kTotalBytes / kSlotBytes,
    kUseAssets = true,
    kValidate = kValidate_
  };

  static_assert(kTotalBytes >= kSlotBytes,
                "PoolAllocator must have enough bytes for one slot. Is "
                "kObjectsPerSlot sensible?");

  using PoolType =
      FixedSizePool<FixedSizePoolParams<kSlotBytes, kNumSlots, alignof(T), 16,
                                        kValidate, kUseAssets>>;
};

template <typename PoolAllocatorConfigT> struct PoolAllocator {
  using T = typename PoolAllocatorConfigT::T;
  using value_type = T;
  using PoolType = typename PoolAllocatorConfigT::PoolType;

  PoolType &m_Pool;

  PoolAllocator(const PoolAllocator &) noexcept = default;
  PoolAllocator &operator=(const PoolAllocator &) = delete;

  PoolAllocator(PoolType &pool) noexcept : m_Pool(pool) {}

  template <typename U> struct rebind {
    using other =
        PoolAllocator<PoolAllocatorConfig<U, PoolAllocatorConfigT::kTotalBytes,
                                          PoolAllocatorConfigT::kObjectsPerSlot,
                                          PoolAllocatorConfigT::kValidate>>;
  };

  T *allocate(std::size_t n);
  void deallocate(T *p, std::size_t) noexcept;
};

template <typename PoolAllocatorConfigT>
typename PoolAllocatorConfigT::T *
PoolAllocator<PoolAllocatorConfigT>::allocate(std::size_t n) {
  if (sizeof(T) * n <= m_Pool.kSlotBytes)
    return (T *)m_Pool.AllocateSlot();

  T *heap = (T *)std::malloc(sizeof(T) * n);
  if (0) {
    XTRACE(MAIN, CRI, "PoolAlloc fallover: %u objs, %u bytes", n,
           sizeof(T) * n);
  }
  return heap;
}

template <typename PoolAllocatorConfigT>
void PoolAllocator<PoolAllocatorConfigT>::deallocate(T *p,
                                                     std::size_t) noexcept {
  if (m_Pool.Contains(p)) {
    m_Pool.DeallocateSlot(p);
  } else {
    std::free(p);
  }
}

template <typename PoolAllocatorConfigT, typename FixedPoolConfigU>
bool operator==(const PoolAllocator<PoolAllocatorConfigT> &,
                const PoolAllocator<FixedPoolConfigU> &) {
  return true;
}

template <typename PoolAllocatorConfigT, typename FixedPoolConfigU>
bool operator!=(const PoolAllocator<PoolAllocatorConfigT> &,
                const PoolAllocator<FixedPoolConfigU> &) {
  return false;
}
