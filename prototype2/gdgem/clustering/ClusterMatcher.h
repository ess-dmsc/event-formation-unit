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

namespace Gem {

class ClusterMatcher {
public:
  explicit ClusterMatcher(uint64_t maxDeltaTime);

  void merge(uint8_t plane, ClusterList &c);

  void match_end(bool force);

  /// \todo match in other ways -- mass, overlap?

  size_t stats_cluster_count{0};

  ClusterList unmatched_clusters;

  std::deque<Event> matched_clusters;

private:
  double pMaxDeltaTime{0};

  bool ready_to_be_matched(uint64_t time) const;

  uint64_t delta_end(const Event &event, const UtpcCluster &cluster) const;
  bool belongs_end(const Event &event, const UtpcCluster &cluster) const;

  uint64_t latest_x{0};
  uint64_t latest_y{0};
};

}
