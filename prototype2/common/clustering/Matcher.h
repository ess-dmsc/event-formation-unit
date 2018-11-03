/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/clustering/GapClusterer.h>
#include <common/clustering/Event.h>
#include <memory>
#include <deque>

namespace Multiblade {

class Matcher {
public:
  explicit Matcher(uint64_t maxDeltaTime);

  void merge(uint8_t plane, ClusterContainer &c);

  void match_end(bool force);

  /// \todo match in other ways -- mass, overlap?

  size_t stats_cluster_count{0};

  ClusterContainer unmatched_clusters;

  std::deque<Event> matched_clusters;

private:
  uint64_t pMaxDeltaTime{0};

  bool ready_to_be_matched(double time) const;

  double delta_end(const Event &event, const Cluster &cluster) const;
  bool belongs_end(const Event &event, const Cluster &cluster) const;

  uint64_t latest_x{0};
  uint64_t latest_y{0};
};

}