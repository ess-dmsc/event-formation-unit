// Copyright (C) 2018-2020 European Spallation Source, ERIC. See LICENSE file
#pragma once

#include <common/Assert.h>
#include <common/FixedSizePool.h>
#include <common/Trace.h>

#include <cstdint>

/// \class PoolAllocatorConfig
/// \brief The class contains the compile-time parameters and configuration for
///        \class PoolAllocator. \class FixedSizePool is used for storage.
template <class T_, size_t TotalBytes_, size_t ObjectsPerSlot_,
          bool Validate_ = true>
struct PoolAllocatorConfig {
  using T = T_;
  enum : size_t {
    TotalBytes = TotalBytes_,
    ObjectsPerSlot = ObjectsPerSlot_,
    SlotBytes = sizeof(T) * ObjectsPerSlot,
    NumSlots = TotalBytes / SlotBytes,
    UseAssets = true,
    Validate = Validate_
  };

  static_assert(TotalBytes >= SlotBytes,
                "PoolAllocator must have enough bytes for one slot. Is "
                "ObjectsPerSlot sensible?");

  using PoolType =
      FixedSizePool<FixedSizePoolParams<SlotBytes, NumSlots, alignof(T), 16,
                                        Validate, UseAssets>>;
};

/// \class PoolAllocator
/// \brief This provides the Allocator interface that STL requires. The user is
///        required to provide a \class FixedSizePool instance to create the
///        allocator object. Config is provided by \class PoolAllocatorConfig.
///        If the FixedSizePool does not have capacity for a requested
///        allocation the allocator will fallback to malloc/free.
template <typename PoolAllocatorConfigT> struct PoolAllocator {
  using T = typename PoolAllocatorConfigT::T;
  using value_type = T;
  using PoolType = typename PoolAllocatorConfigT::PoolType;

  PoolType &Pool;

  PoolAllocator(const PoolAllocator &) noexcept = default;
  PoolAllocator &operator=(const PoolAllocator &) = delete;

  PoolAllocator(PoolType &pool) noexcept : Pool(pool) {}

  template <typename U> struct rebind {
    using other =
        PoolAllocator<PoolAllocatorConfig<U, PoolAllocatorConfigT::TotalBytes,
                                          PoolAllocatorConfigT::ObjectsPerSlot,
                                          PoolAllocatorConfigT::Validate>>;
  };

  T *allocate(std::size_t n);
  void deallocate(T *p, std::size_t) noexcept;
};

template <typename PoolAllocatorConfigT>
typename PoolAllocatorConfigT::T *
PoolAllocator<PoolAllocatorConfigT>::allocate(std::size_t n) {
  size_t byteCount = sizeof(T) * n;
  T *bytes = nullptr;
  if (__builtin_expect(byteCount <= Pool.SlotBytes, 1)) {
    bytes = (T *)Pool.AllocateSlot(byteCount);
  }
  if (__builtin_expect(bytes == nullptr, 0)) {
    bytes = (T *)std::malloc(byteCount);
    if (0) {
      XTRACE(MAIN, CRI, "PoolAlloc fallover: %u objs, %u bytes", n, byteCount);
    }
  }
  return bytes;
}

template <typename PoolAllocatorConfigT>
void PoolAllocator<PoolAllocatorConfigT>::deallocate(T *p,
                                                     std::size_t) noexcept {
  if (__builtin_expect(Pool.Contains(p), 1)) {
    Pool.DeallocateSlot(p);
  } else {
    std::free(p);
  }
}

template <typename PoolAllocatorConfigT, typename PoolAllocatorConfigU>
bool operator==(const PoolAllocator<PoolAllocatorConfigT> &,
                const PoolAllocator<PoolAllocatorConfigU> &) {
  return true;
}

template <typename PoolAllocatorConfigT, typename PoolAllocatorConfigU>
bool operator!=(const PoolAllocator<PoolAllocatorConfigT> &,
                const PoolAllocator<PoolAllocatorConfigU> &) {
  return false;
}
