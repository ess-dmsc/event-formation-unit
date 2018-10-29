/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <gdgem/clustering/AbstractClusterer.h>
#include <deque>

class ClusterMatcher {
public:
  explicit ClusterMatcher(double maxDeltaTime);

  void merge(uint8_t plane, ClusterList& c);

  void match_end(bool force);

  /// \todo match in other ways -- mass, overlap?

  size_t stats_cluster_count {0};

  ClusterList unmatched_clusters;

  std::deque<Event> matched_clusters;

private:
  double pMaxDeltaTime {0};

  bool ready_to_be_matched(double time) const;

  double delta_end(const Event& event, const Cluster& cluster) const;
  bool belongs_end(const Event& event, const Cluster& cluster) const;

  double latest_x {0};
  double latest_y {0};
};
