// Copyright (C) 2018-2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file GapClusterer2D.h
/// \brief GapClusterer2D class definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/reduction/clustering/AbstractClusterer.h>
#include <common/reduction/multigrid/ModuleGeometry.h>

/// \todo update documentation for 2D version

/// \class GapClusterer2D GapClusterer2D.h
/// \brief Clusterer for hits in one plane, discriminating clusters
///         based on predefined gaps in time and space. Supplied hits must be
///         chronologically sorted within supplied containers and between
///         subsequent instances thereof. Clustering is first performed in time,
///         and then in space.

class GapClusterer2D : public AbstractClusterer {
public:
  /// \brief GapClusterer2D constructor
  /// \param max_time_gap maximum difference in time between hits such that
  ///        they would be considered part of the same cluster
  /// \param max_coord_gap maximum difference in coordinates between hits such
  ///        that they would be considered part of the same cluster
  GapClusterer2D(uint64_t max_time_gap, uint16_t max_coord_gap);

  /// \param geom sets the ModuleGeometry definition for converting Wires to X
  /// and Z
  void set_geometry(const Multigrid::ModuleGeometry &geom);

  /// \returns current ModuleGeometry definition
  Multigrid::ModuleGeometry geometry() const;

  /// \brief insert new hit and perform clustering
  /// \param hit to be added to cluster. Hits must be chronological between
  ///         subsequent calls. It may be more efficient to use:
  /// \sa GapClusterer2D::cluster
  void insert(const Hit &hit) override;

  /// \brief insert new hits and perform clustering
  /// \param hits container of hits to be processed. Hits must be
  /// chronologically
  ///        sorted within the container and between subsequent calls.
  void cluster(const HitVector &hits) override;

  /// \brief complete clustering for any remaining hits
  void flush() override;

  /// \brief print configuration of GapClusterer2D
  std::string config(const std::string &prepend) const override;

  /// \brief print current status of GapClusterer2D
  std::string status(const std::string &prepend, bool verbose) const override;

private:
  uint64_t max_time_gap_;
  uint16_t max_coord_gap_;

  HitVector
      current_time_cluster_; ///< kept in memory until time gap encountered

  Multigrid::ModuleGeometry geometry_;

  /// \brief helper function to clusters hits in current_time_cluster_
  void cluster_by_x();

  void cluster_by_z(HitVector &x_cluster);

  void stash_cluster(HitVector &xz_cluster);

  inline void sort_by_x(HitVector &hits) {
    std::sort(hits.begin(), hits.end(),
              [this](const Hit &hit1, const Hit &hit2) {
                return geometry_.x_from_wire(hit1.coordinate) <
                       geometry_.x_from_wire(hit2.coordinate);
              });
  }

  inline void sort_by_z(HitVector &hits) {
    std::sort(hits.begin(), hits.end(),
              [this](const Hit &hit1, const Hit &hit2) {
                return geometry_.z_from_wire(hit1.coordinate) <
                       geometry_.z_from_wire(hit2.coordinate);
              });
  }
};
