/** Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file **/
//===----------------------------------------------------------------------===//
///
/// \file GapClusterer.h
/// \brief GapClusterer class definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/clustering/AbstractClusterer.h>

/// \class GapClusterer GapClusterer.h
/// \brief Clusterer for hits in one plane, discriminating clusters
///         based on predefined gaps in time and space. Supplied hits must be
///         chronologically sorted within supplied containers and between
///         subsequent instances thereof. Clustering is first performed in time,
///         and then in space.

class GapClusterer : public AbstractClusterer {
public:
  /// \brief GapClusterer constructor
  /// \param max_time_gap maximum difference in time between hits such that
  ///        they would be considered part of the same cluster
  /// \param max_coord_gap maximum difference in coordinates between hits such
  ///        that they would be considered part of the same cluster
  GapClusterer(uint64_t max_time_gap, uint16_t max_coord_gap);

  /// \brief insert new hit and perform clustering
  /// \param hit to be added to cluster. Hits must be chronological between
  ///         subsequent calls. It may be more efficient to use:
  /// \sa GapClusterer::cluster
  void insert(const Hit &hit) override;

  /// \brief insert new hits and perform clustering
  /// \param hits container of hits to be processed. Hits must be chronologically
  ///        sorted within the container and between subsequent calls.
  void cluster(const HitVector &hits) override;

  /// \brief complete clustering for any remaining hits
  void flush() override;

  /// \brief print configuration of GapClusterer
  std::string config(const std::string &prepend) const override;

  /// \brief print current status of GapClusterer
  std::string status(const std::string &prepend, bool verbose) const override;

private:
  uint64_t max_time_gap_;
  uint16_t max_coord_gap_;

  HitVector current_time_cluster_; ///< kept in memory until time gap encountered

  /// \brief helper function to clusters hits in current_time_cluster_
  void cluster_by_coordinate();
};
