/** Copyright (C) 2018-2019 European Spallation Source, ERIC. See LICENSE file **/
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

// \todo the abstract class code needs tests

// \todo refactor: move out to separate header
// \todo replace by deque, or....?
using ClusterContainer = std::list<Cluster>;

/// \brief convenience function for printing a ClusterContainer
std::string to_string(const ClusterContainer &container,
                      const std::string &prepend,
                      bool verbose);

/// \class AbstractClusterer AbstractClusterer.h
/// \brief AbstractClusterer declares the interface for a clusterer class
///         that should group hits into clusters. Provides base functionality
///         for storage of clusters and stats counter. Other pre- and
///         post-conditions are left to the discretion of a specific
///         implementation.

class AbstractClusterer {
public:
  ClusterContainer clusters;     ///< clustered hits
  mutable size_t stats_cluster_count{0}; ///< cumulative number of clusters produced

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
