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
//#include <deque>

struct ClusterContainerAllocatorBase {
  static char *s_MemBegin;
  static char *s_MemEnd;
};

template <class T>
struct ClusterContainerAllocator : public ClusterContainerAllocatorBase {
  typedef T value_type;

  ClusterContainerAllocator() = default;
  template <class U>
  constexpr ClusterContainerAllocator(
      const ClusterContainerAllocator<U> &) noexcept {}

  T *allocate(std::size_t n) {
    char *p = s_MemBegin;
    s_MemBegin += n * sizeof(T);
    if (s_MemBegin < s_MemEnd)
      return (T *)p;
    return nullptr;
    // throw std::bad_alloc();
  }
  void deallocate(T *, std::size_t) noexcept { /* do nothing*/
  }
};

template <class T, class U>
bool operator==(const ClusterContainerAllocator<T> &,
                const ClusterContainerAllocator<U> &) {
  return true;
}
template <class T, class U>
bool operator!=(const ClusterContainerAllocator<T> &,
                const ClusterContainerAllocator<U> &) {
  return false;
}

//-----------------------------------------------------------------------------

struct ClusterPoolStorage {
  struct ClusterAndlListNodeGuess {
    Cluster cluster;
    Cluster listNodeGuess;
  };
  using PoolCfg =
      FixedPoolConfig<ClusterAndlListNodeGuess, 1024 * 1024 * 1024, 1>;
  static PoolCfg::PoolType* s_Pool;
  static PoolAllocator<PoolCfg> s_Alloc;
};

template <class T> struct ClusterPoolAllocator : public ClusterPoolStorage {
  using value_type = T;

  template <typename U> struct rebind {
    using other = ClusterPoolAllocator<U>;
  };

  ClusterPoolAllocator() = default;

  // needed to convert from internal node alloc to cluster alloc.
  template <class U>
  constexpr ClusterPoolAllocator(const ClusterPoolAllocator<U> &) noexcept {}

  T *allocate(std::size_t n) {
    static_assert(sizeof(T) <= sizeof(ClusterAndlListNodeGuess),
                  "ClusterAndlListNodeGuess needs more space");
    static_assert(alignof(T) <= alignof(ClusterAndlListNodeGuess),
                  "ClusterAndlListNodeGuess needs higher align");

    RelAssertMsg(n == 1, "not expecting bulk allocation from std::list");
    //if (!std::is_same<T, Cluster>::value) {
    //  XTRACE(MAIN, CRI, "hello node type");
    //}
    return (T *)s_Alloc.allocate(1);
  }

  void deallocate(T *p, std::size_t n) noexcept {
    s_Alloc.deallocate((ClusterAndlListNodeGuess *)p, n);
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
