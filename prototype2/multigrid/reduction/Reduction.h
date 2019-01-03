/** Copyright (C) 2016-2018 European Spallation Source */

#pragma once
#include <common/clustering/GapClusterer.h>
#include <common/clustering/GapMatcher.h>

namespace Multigrid {

class Reduction {
public:

  void ingest(HitContainer &hits);

  void perform_clustering(bool flush);

  size_t stats_invalid_planes{0};
  size_t stats_time_seq_errors{0};

  // \todo encapsulate this properly
  GapClusterer wire_clusters{0, 1};
  GapClusterer grid_clusters{0, 1};
  HitContainer pulse_times;
  // Just greater than shortest pulse period of 266662 ticks
  // Will have to be adjusted for other experimental setups
  GapMatcher matcher{300000, 1};

private:
  uint64_t previous_time_{0};

};

}