/** Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file **/
//===----------------------------------------------------------------------===//
///
/// \file GapClusterer.cpp
/// \brief GapClusterer class implementation
///
//===----------------------------------------------------------------------===//

#include <common/clustering/GapClusterer.h>
#include <algorithm>

GapClusterer::GapClusterer(uint64_t max_time_gap, uint16_t max_coord_gap)
    : AbstractClusterer()
    , max_time_gap_(max_time_gap)
    , max_coord_gap_(max_coord_gap) {}

void GapClusterer::cluster(const HitContainer &hits) {
  //It is assumed that hits are sorted in time

  for (const auto &hit : hits) {
    // Stash cluster if time gap to next hit is too large
    if (!current_time_cluster_.empty() &&
        (hit.time - current_time_cluster_.back().time) > max_time_gap_) {
      flush();
    }

    // Insert in either case
    current_time_cluster_.emplace_back(hit);
  }
}

void GapClusterer::flush() {
  cluster_by_coordinate();
  current_time_cluster_.clear();
}

void GapClusterer::cluster_by_coordinate() {
  // First, sort in terms of coordinate
  std::sort(current_time_cluster_.begin(), current_time_cluster_.end(),
            [](const Hit &e1, const Hit &e2) {
              return e1.coordinate < e2.coordinate;
            });

  Cluster cluster;
  for (auto &hit : current_time_cluster_) {
    // Stash cluster if coordinate gap to next hit is too large
    if (!cluster.empty() &&
        (hit.coordinate - cluster.coord_end()) > max_coord_gap_) {
      stash_cluster(cluster);
      cluster.clear();
    }

    // insert in either case
    cluster.insert(hit);
  }

  // Attempt to stash any leftovers
  stash_cluster(cluster);
}
