// Copyright (C) 2018 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file GapClusterer2D.cpp
/// \brief GapClusterer2D class implementation
///
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <common/reduction/clustering/GapClusterer2D.h>
#include <fmt/format.h>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

GapClusterer2D::GapClusterer2D(uint64_t max_time_gap, uint16_t max_coord_gap)
    : AbstractClusterer(), max_time_gap_(max_time_gap),
      max_coord_gap_(max_coord_gap) {}

void GapClusterer2D::set_geometry(const Multigrid::ModuleGeometry &geom) {
  geometry_ = geom;
}

Multigrid::ModuleGeometry GapClusterer2D::geometry() const { return geometry_; }

void GapClusterer2D::insert(const Hit &hit) {
  /// Process time-cluster if time gap to next hit is large enough
  if (!current_time_cluster_.empty() &&
      (hit.time - current_time_cluster_.back().time) > max_time_gap_) {
    flush();
  }

  /// Insert hit in either case
  current_time_cluster_.emplace_back(hit);
}

void GapClusterer2D::cluster(const HitVector &hits) {
  /// It is assumed that hits are sorted in time
  for (const auto &hit : hits) {
    insert(hit);
  }
}

void GapClusterer2D::flush() {
  XTRACE(EVENT, DEB, "Flushing clusterer");
  if (current_time_cluster_.empty()) {
    return;
  }
  cluster_by_x();
  current_time_cluster_.clear();
}

void GapClusterer2D::cluster_by_x() {
  /// First, sort in terms of coordinate
  sort_by_x(current_time_cluster_);

  HitVector x_cluster;

  for (auto &hit : current_time_cluster_) {
    /// Stash cluster if coordinate gap to next hit is too large
    if (!x_cluster.empty() &&
        (geometry_.x_from_wire(hit.coordinate) -
         geometry_.x_from_wire(x_cluster.back().coordinate)) > max_coord_gap_) {
      cluster_by_z(x_cluster);
      x_cluster.clear();
    }

    /// insert in either case
    x_cluster.push_back(hit);
  }

  /// Stash any leftovers
  if (!x_cluster.empty())
    cluster_by_z(x_cluster);
}

void GapClusterer2D::cluster_by_z(HitVector &x_cluster) {
  /// First, sort in terms of coordinate
  sort_by_z(x_cluster);

  HitVector z_cluster;

  for (auto &hit : x_cluster) {
    /// Stash cluster if coordinate gap to next hit is too large

    if (!z_cluster.empty() &&
        (geometry_.z_from_wire(hit.coordinate) -
         geometry_.z_from_wire(z_cluster.back().coordinate)) > max_coord_gap_) {
      stash_cluster(z_cluster);
      z_cluster.clear();
    }

    /// insert in either case
    z_cluster.push_back(hit);
  }

  /// Stash any leftovers
  if (!z_cluster.empty())
    stash_cluster(z_cluster);
}

void GapClusterer2D::stash_cluster(HitVector &xz_cluster) {
  Cluster cluster;
  for (const auto &hit : xz_cluster) {
    cluster.insert(hit);
  }
  AbstractClusterer::stash_cluster(cluster);
}

std::string GapClusterer2D::config(const std::string &prepend) const {
  std::stringstream ss;
  ss << "GapClusterer2D:\n";
  ss << prepend << fmt::format("max_time_gap={}\n", max_time_gap_);
  ss << prepend << fmt::format("max_coord_gap={}\n", max_coord_gap_);
  ss << prepend << geometry_.debug(prepend + "  ");
  return ss.str();
}

std::string GapClusterer2D::status(const std::string &prepend,
                                   bool verbose) const {
  std::stringstream ss;
  ss << AbstractClusterer::status(prepend, verbose);
  if (!current_time_cluster_.empty())
    ss << prepend << "Current time cluster:\n"
       << to_string(current_time_cluster_, prepend + "  ") + "\n";
  return ss.str();
}
