// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file Abstract2DClusterer.h
/// \brief Abstract2DClusterer class definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/reduction/Cluster2D.h>
#include <list>

#define ENABLE_GREEDY_CLUSTER_ALLOCATOR 0

struct GreedyCluster2DStorage {
  static char *MemBegin;
  static char *MemEnd;
};

template <class T> struct GreedyCluster2DAllocator {
  typedef T value_type;

  GreedyCluster2DAllocator() = default;
  template <class U>
  constexpr GreedyCluster2DAllocator(const GreedyCluster2DAllocator<U> &) noexcept {
  }

  T *allocate(std::size_t n) {
    RelAssertMsg(ENABLE_GREEDY_CLUSTER_ALLOCATOR, "Remember to enable");
    char *p = GreedyCluster2DStorage::MemBegin;
    GreedyCluster2DStorage::MemBegin += n * sizeof(T);
    if (GreedyCluster2DStorage::MemBegin < GreedyCluster2DStorage::MemEnd)
      return (T *)p;
    throw std::bad_alloc();
  }
  void deallocate(T *, std::size_t) noexcept { // do nothing
  }
};

template <class T, class U>
bool operator==(const GreedyCluster2DAllocator<T> &,
                const GreedyCluster2DAllocator<U> &) {
  return true;
}
template <class T, class U>
bool operator!=(const GreedyCluster2DAllocator<T> &,
                const GreedyCluster2DAllocator<U> &) {
  return false;
}

//-----------------------------------------------------------------------------

struct Cluster2DPoolStorage {
  struct StorageGuess {
    Cluster2D cluster;
    void *nodeGuess1;
    void *nodeGuess2;
  };
  enum : size_t { Bytes_1GB = 1024 * 1024 * 1024, ObjectsPerSlot = 1 };
  using AllocConfig =
      PoolAllocatorConfig<StorageGuess, Bytes_1GB, ObjectsPerSlot, false, true>;
  static AllocConfig::PoolType *Pool;
  static PoolAllocator<AllocConfig> Alloc;
};

template <class T> struct Cluster2DPoolAllocator {
  using value_type = T;

  // checks due to std::list::__node_type is implementation defined
  static_assert(sizeof(T) <= sizeof(Cluster2DPoolStorage::StorageGuess),
                "StorageGuess needs more space");
  static_assert(alignof(T) <= alignof(Cluster2DPoolStorage::StorageGuess),
                "StorageGuess needs higher align");

  template <typename U> struct rebind {
    using other = Cluster2DPoolAllocator<U>;
  };

  Cluster2DPoolAllocator() = default;

  // needed to convert from internal node alloc to cluster alloc.
  template <class U>
  constexpr Cluster2DPoolAllocator(const Cluster2DPoolAllocator<U> &) noexcept {}

  T *allocate(std::size_t n) {
    RelAssertMsg(n == 1, "not expecting bulk allocation from std::list");
    // if (!std::is_same<T, Cluster2D>::value) XTRACE(MAIN, CRI, "node");
    return (T *)Cluster2DPoolStorage::Alloc.allocate(1);
  }

  void deallocate(T *p, std::size_t n) noexcept {
    Cluster2DPoolStorage::Alloc.deallocate((Cluster2DPoolStorage::StorageGuess *)p,
                                         n);
  }
};

template <class T, class U>
bool operator==(const Cluster2DPoolAllocator<T> &,
                const Cluster2DPoolAllocator<U> &) {
  return true;
}
template <class T, class U>
bool operator!=(const Cluster2DPoolAllocator<T> &,
                const Cluster2DPoolAllocator<U> &) {
  return false;
}

//-----------------------------------------------------------------------------

/// \todo the abstract class code needs tests

/// \todo refactor: move out to separate header
/// \todo replace by deque, or....?
// using Cluster2DContainer = std::list<Cluster2D,
// Cluster2DContainerAllocator<Cluster2D>>;
// using Cluster2DContainer = std::list<Cluster2D>;
using Cluster2DContainer = std::list<Cluster2D, Cluster2DPoolAllocator<Cluster2D>>;

/// \brief convenience function for printing a Cluster2DContainer
std::string to_string(const Cluster2DContainer &container,
                      const std::string &prepend, bool verbose);

/// \class Abstract2DClusterer Abstract2DClusterer.h
/// \brief Abstract2DClusterer declares the interface for a clusterer class
///         that should group hits into clusters. Provides base functionality
///         for storage of clusters and stats counter. Other pre- and
///         post-conditions are left to the discretion of a specific
///         implementation.

class Abstract2DClusterer {
public:
  Cluster2DContainer clusters; ///< clustered hits
  mutable size_t stats_cluster_count{
      0}; ///< cumulative number of clusters produced

public:
  Abstract2DClusterer() = default;
  virtual ~Abstract2DClusterer() = default;

  /// \brief inserts new hit and potentially performs some clustering
  virtual void insert(const Hit2D &hit) = 0;

  /// \brief inserts new hits and potentially performs some clustering
  virtual void cluster(const Hit2DVector &hits) = 0;

  /// \brief complete clustering for any remaining hits
  virtual void flush() = 0;

  /// \brief print configuration of Clusterer
  virtual std::string config(const std::string &prepend) const = 0;

  /// \brief print current status of Clusterer
  virtual std::string status(const std::string &prepend, bool verbose) const;

  /// \brief convenience function
  /// \returns if cluster container is empty
  bool empty() const;

protected:
  /// \brief moves cluster into clusters container, increments counter
  /// \param cluster to be stashed
  void stash_cluster(Cluster2D &cluster);
};
