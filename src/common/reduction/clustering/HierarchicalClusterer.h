// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file HierarchicalClusterer.h
/// \brief HierarchicalClusterer class definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/reduction/clustering/Abstract2DClusterer.h>
#include <common/reduction/multigrid/ModuleGeometry.h>

/// \todo update documentation for 2D version

/// \class HierarchicalClusterer HierarchicalClusterer.h
/// \brief Clusterer for 2D hits in time, and then space using a euclidian distance measure
///         2D hits differ from standard hits in that we already know both the x and y location
///         of the hit

class HierarchicalClusterer : public Abstract2DClusterer {
public:
  /// \brief HierarchicalClusterer constructor
  /// \param max_time_gap maximum difference in time between hits such that
  ///        they would be considered part of the same cluster
  /// \param max_coord_gap maximum difference in coordinates between hits such
  ///        that they would be considered part of the same cluster
  HierarchicalClusterer(uint64_t max_time_gap, uint16_t max_coord_gap);

   /// \brief insert new hit and perform clustering
  /// \param hit to be added to cluster. Hits must be chronological between
  ///         subsequent calls. It may be more efficient to use:
  /// \sa HierarchicalClusterer::cluster
  void insert(const Hit2D &hit) override;

  /// \brief insert new hits and perform clustering
  /// \param hits container of hits to be processed. Hit2Ds must be
  /// chronologically
  ///        sorted within the container and between subsequent calls.
  void cluster(const Hit2DVector &hits) override;

  /// \brief complete clustering for any remaining hits
  void flush() override;

  /// \brief print configuration of HierarchicalClusterer
  std::string config(const std::string &prepend) const override;

  /// \brief print current status of HierarchicalClusterer
  std::string status(const std::string &prepend, bool verbose) const override;

private:
  uint64_t max_time_gap_;
  uint16_t max_coord_gap_;

  Hit2DVector
      current_time_cluster_; ///< kept in memory until time gap encountered

  /// \brief helper function to clusters hits in current_time_cluster_
  void cluster_by_x();

  void stash_cluster(Hit2DVector &xz_cluster);

};
