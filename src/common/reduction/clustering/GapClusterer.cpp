// Copyright (C) 2018-2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file GapClusterer.cpp
/// \brief GapClusterer class implementation
///
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <common/reduction/clustering/GapClusterer.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

GapClusterer::GapClusterer(uint64_t max_time_gap, uint16_t max_coord_gap)
    : AbstractClusterer(), max_time_gap_(max_time_gap),
      max_coord_gap_(max_coord_gap) {}

void GapClusterer::insert(const Hit &hit) {
  /// Process time-cluster if time gap to next hit is large enough
  if (!current_time_cluster_.empty() &&
      (hit.time - current_time_cluster_.back().time) > max_time_gap_) {
    XTRACE(CLUSTER, DEB, "timegap > %lu, hit: %lu, current: %lu", max_time_gap_,
           hit.time, current_time_cluster_.back().time);
    flush();
  }
  else{
    XTRACE(CLUSTER, DEB, "timegap < %lu, hit: %lu, current: %lu", max_time_gap_,
           hit.time, current_time_cluster_.back().time);
    XTRACE(CLUSTER, DEB, "current time cluster length: %u", current_time_cluster_.size());
  }

  /// Insert hit in either case
  XTRACE(CLUSTER, DEB, "insert hit %s", hit.to_string().c_str());
  current_time_cluster_.emplace_back(hit);
}

void GapClusterer::cluster(const HitVector &hits) {
  /// It is assumed that hits are sorted in time
  for (const auto &hit : hits) {
    XTRACE(CLUSTER, DEB, "insert hit %s", hit.to_string().c_str());
    insert(hit);
  }
}

void GapClusterer::flush() {
  XTRACE(CLUSTER, DEB, "flushing clusterer");
  if (current_time_cluster_.empty()) {
    return;
  }
  cluster_by_coordinate();
  current_time_cluster_.clear();
}

void GapClusterer::cluster_by_coordinate() {
  /// First, sort in terms of coordinate
  sort_by_increasing_coordinate(current_time_cluster_);

  Cluster cluster;
  XTRACE(CLUSTER, DEB, "cur time cluster: first coord %u, last coord %u",
         current_time_cluster_.front().coordinate,
         current_time_cluster_.back().coordinate);

  for (auto &hit : current_time_cluster_) {
    /// Stash cluster if coordinate gap to next hit is too large
    XTRACE(CLUSTER, DEB, "hit coord %u, cluster coord end %u", hit.coordinate,
           cluster.coord_end());

    if (!cluster.empty() &&
        (hit.coordinate - cluster.coord_end()) > max_coord_gap_) {
      XTRACE(CLUSTER, DEB,
             "Stashing cluster - max_coord_gap exceeded (%i > %i)",
             hit.coordinate - cluster.coord_end(), max_coord_gap_);
      stash_cluster(cluster);
      cluster.clear();
    }

    /// insert in either case
    XTRACE(CLUSTER, DEB, "insert hit=%s", hit.to_string().c_str());
    cluster.insert(hit);
  }

  /// Stash any leftovers
  if (!cluster.empty())
    stash_cluster(cluster);
}

std::string GapClusterer::config(const std::string &prepend) const {
  std::stringstream ss;
  ss << "GapClusterer:\n";
  ss << prepend << fmt::format("max_time_gap={}\n", max_time_gap_);
  ss << prepend << fmt::format("max_coord_gap={}\n", max_coord_gap_);
  return ss.str();
}

std::string GapClusterer::status(const std::string &prepend,
                                 bool verbose) const {
  std::stringstream ss;
  ss << AbstractClusterer::status(prepend, verbose);
  if (!current_time_cluster_.empty())
    ss << prepend << "Current time cluster:\n"
       << to_string(current_time_cluster_, prepend + "  ") + "\n";
  return ss.str();
}
