/*Copyright (C) 2018-2019 European Spallation Source, ERIC. See LICENSE file*/
//===----------------------------------------------------------------------===//
///
/// \file AbstractClusterer.h
/// \brief AbstractClusterer class definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/reduction/Cluster.h>
#include <list>

#define ENABLE_GREEDY_CLUSTER_ALLOCATOR 0

struct GreedyClusterStorage {
  static char *MemBegin;
  static char *MemEnd;
};

template <class T> struct GreedyClusterAllocator {
  typedef T value_type;

  GreedyClusterAllocator() = default;
  template <class U>
  constexpr GreedyClusterAllocator(const GreedyClusterAllocator<U> &) noexcept {
  }

  T *allocate(std::size_t n) {
    RelAssertMsg (ENABLE_GREEDY_CLUSTER_ALLOCATOR, "Remember to enable");
    char *p = GreedyClusterStorage::MemBegin;
    GreedyClusterStorage::MemBegin += n * sizeof(T);
    if (GreedyClusterStorage::MemBegin < GreedyClusterStorage::MemEnd)
      return (T *)p;
    throw std::bad_alloc();
  }
  void deallocate(T *, std::size_t) noexcept { /* do nothing*/
  }
};

template <class T, class U>
bool operator==(const GreedyClusterAllocator<T> &,
                const GreedyClusterAllocator<U> &) {
  return true;
}
template <class T, class U>
bool operator!=(const GreedyClusterAllocator<T> &,
                const GreedyClusterAllocator<U> &) {
  return false;
}

//-----------------------------------------------------------------------------

struct ClusterPoolStorage {
  struct StorageGuess {
    Cluster cluster;
    void *nodeGuess1;
    void *nodeGuess2;
  };
  enum : size_t { Bytes_1GB = 1024 * 1024 * 1024, ObjectsPerSlot = 1 };
  using AllocConfig =
      PoolAllocatorConfig<StorageGuess, Bytes_1GB, ObjectsPerSlot, false>;
  static AllocConfig::PoolType *Pool;
  static PoolAllocator<AllocConfig> Alloc;
};

template <class T> struct ClusterPoolAllocator {
  using value_type = T;

  // checks due to std::list::__node_type is implementation defined
  static_assert(sizeof(T) <= sizeof(ClusterPoolStorage::StorageGuess),
                "StorageGuess needs more space");
  static_assert(alignof(T) <= alignof(ClusterPoolStorage::StorageGuess),
                "StorageGuess needs higher align");

  template <typename U> struct rebind {
    using other = ClusterPoolAllocator<U>;
  };

  ClusterPoolAllocator() = default;

  // needed to convert from internal node alloc to cluster alloc.
  template <class U>
  constexpr ClusterPoolAllocator(const ClusterPoolAllocator<U> &) noexcept {}

  T *allocate(std::size_t n) {
    RelAssertMsg(n == 1, "not expecting bulk allocation from std::list");
    // if (!std::is_same<T, Cluster>::value) XTRACE(MAIN, CRI, "node");
    return (T *)ClusterPoolStorage::Alloc.allocate(1);
  }

  void deallocate(T *p, std::size_t n) noexcept {
    ClusterPoolStorage::Alloc.deallocate((ClusterPoolStorage::StorageGuess *)p,
                                         n);
  }
};

template <class T, class U>
bool operator==(const ClusterPoolAllocator<T> &,
                const ClusterPoolAllocator<U> &) {
  return true;
}
template <class T, class U>
bool operator!=(const ClusterPoolAllocator<T> &,
                const ClusterPoolAllocator<U> &) {
  return false;
}

//-----------------------------------------------------------------------------

// \todo the abstract class code needs tests

// \todo refactor: move out to separate header
// \todo replace by deque, or....?
// using ClusterContainer = std::list<Cluster,
// ClusterContainerAllocator<Cluster>>;
// using ClusterContainer = std::list<Cluster>;
using ClusterContainer = std::list<Cluster, ClusterPoolAllocator<Cluster>>;

/// \brief convenience function for printing a ClusterContainer
std::string to_string(const ClusterContainer &container,
                      const std::string &prepend, bool verbose);

/// \class AbstractClusterer AbstractClusterer.h
/// \brief AbstractClusterer declares the interface for a clusterer class
///         that should group hits into clusters. Provides base functionality
///         for storage of clusters and stats counter. Other pre- and
///         post-conditions are left to the discretion of a specific
///         implementation.

class AbstractClusterer {
public:
  ClusterContainer clusters; ///< clustered hits
  mutable size_t stats_cluster_count{
      0}; ///< cumulative number of clusters produced

public:
  AbstractClusterer() = default;
  virtual ~AbstractClusterer() = default;

  /// \brief inserts new hit and potentially performs some clustering
  virtual void insert(const Hit &hit) = 0;

  /// \brief inserts new hits and potentially performs some clustering
  virtual void cluster(const HitVector &hits) = 0;

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
  void stash_cluster(Cluster &cluster);
};
