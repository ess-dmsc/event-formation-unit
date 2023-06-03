// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file HierarchicalClusterer.cpp
/// \brief HierarchicalClusterer class implementation
///
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <common/reduction/clustering/HierarchicalClusterer.h>
#include <set>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

HierarchicalClusterer::HierarchicalClusterer(uint64_t max_time_gap, uint16_t max_coord_gap)
    : Abstract2DClusterer(), max_time_gap_(max_time_gap),
      max_coord_gap_(max_coord_gap) {}


void HierarchicalClusterer::insert(const Hit2D &hit) {
  /// Process time-cluster if time gap to next hit is large enough
  if (!current_time_cluster_.empty() &&
      (hit.time - current_time_cluster_.back().time) > max_time_gap_) {
    flush();
  }

  /// Insert hit in either case
  current_time_cluster_.emplace_back(hit);
}

void HierarchicalClusterer::cluster(const Hit2DVector &hits) {
  /// It is assumed that hits are sorted in time
  for (const auto &hit : hits) {
    insert(hit);
  }
}

void HierarchicalClusterer::flush() {
  XTRACE(EVENT, DEB, "Flushing clusterer");
  if (current_time_cluster_.empty()) {
    return;
  }
  cluster_by_x();
  current_time_cluster_.clear();
}

void HierarchicalClusterer::cluster_by_x() {
  uint clusterSize = current_time_cluster_.size();
  std::vector<bool> visited(clusterSize, false); // keep track of visited points
  
  XTRACE(DATA, DEB, "%u events in time window", clusterSize);
  for (uint i = 0; i < clusterSize; i++) {
    if (visited[i]) {
      continue; // skip points that have already been visited
    }
    XTRACE(DATA, DEB, "Starting new cluster");
    Hit2DVector space_cluster; // initialize a new cluster
    space_cluster.push_back(current_time_cluster_[i]);
    visited[i] = true;
    for (uint j = i+1; j < clusterSize; j++) {
      if (visited[j]) {
        continue; // skip points that have already been visited
      }
      double x_distance = (double)current_time_cluster_[i].x_coordinate - (double)current_time_cluster_[j].x_coordinate;
      double y_distance = (double)current_time_cluster_[i].y_coordinate - (double)current_time_cluster_[j].y_coordinate;
      double distance = sqrt(pow(x_distance, 2) + pow(y_distance, 2));
      XTRACE(DATA, DEB, "Determined distance between points is %f, threshold is %u", distance, max_coord_gap_);
      XTRACE(DATA, DEB, "X1 = %u, X2 = %u, Y1 = %u, Y2 = %u", current_time_cluster_[i].x_coordinate, current_time_cluster_[j].x_coordinate, current_time_cluster_[i].y_coordinate, current_time_cluster_[j].y_coordinate);
      if (distance < max_coord_gap_) {
        XTRACE(DATA, DEB, "Adding to existing cluster");
        space_cluster.push_back(current_time_cluster_[j]); // add point to current cluster
        visited[j] = true;
      }
      else{
        XTRACE(DATA, DEB, "Too far apart, not including in this cluster");
      }
    }
    stash_cluster(space_cluster); // add completed cluster to list of clusters
  }
}


void HierarchicalClusterer::stash_cluster(Hit2DVector &xz_cluster) {
  Cluster2D cluster;
  for (const auto &hit : xz_cluster) {
    cluster.insert(hit);
  }
  Abstract2DClusterer::stash_cluster(cluster);
}

std::string HierarchicalClusterer::config(const std::string &prepend) const {
  std::stringstream ss;
  ss << "HierarchicalClusterer:\n";
  ss << prepend << fmt::format("max_time_gap={}\n", max_time_gap_);
  ss << prepend << fmt::format("max_coord_gap={}\n", max_coord_gap_);
  return ss.str();
}

std::string HierarchicalClusterer::status(const std::string &prepend,
                                   bool verbose) const {
  std::stringstream ss;
  ss << Abstract2DClusterer::status(prepend, verbose);
  if (!current_time_cluster_.empty())
    ss << prepend << "Current time cluster:\n"
       << to_string(current_time_cluster_, prepend + "  ") + "\n";
  return ss.str();
}
