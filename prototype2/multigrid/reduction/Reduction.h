/** Copyright (C) 2016-2018 European Spallation Source */

#pragma once
#include <common/clustering/GapClusterer.h>
#include <common/clustering/GapMatcher.h>
#include <common/reduction/MgAnalyzer.h>

namespace Multigrid {

class Reduction {
public:
  Reduction();

  void ingest(HitVector &hits);

  void ingest(const Hit& hit);

  void perform_clustering(bool flush);

  size_t stats_invalid_planes{0};
  size_t stats_time_seq_errors{0};
  size_t stats_wire_clusters{0};
  size_t stats_grid_clusters{0};

  // Just greater than shortest pulse period of 266662 ticks
  // Will have to be adjusted for other experimental setups
  GapMatcher matcher{300000, 0, 1, Hit::PulsePlane};

private:
  GapClusterer wire_clusters{0, 1};
  GapClusterer grid_clusters{0, 1};
  HitVector pulse_times;

  uint64_t previous_time_{0};
};

}