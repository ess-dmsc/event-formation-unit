/** Copyright (C) 2016-2018 European Spallation Source */

#pragma once
#include <common/clustering/GapClusterer.h>
#include <common/clustering/GapMatcher.h>

namespace Multigrid {

class Reduction {
public:

  void ingest(const HitContainer& hits);

  void perform_clustering(bool flush);

  void flush();

  size_t stats_invalid_planes{0};
  size_t stats_time_seq_errors{0};

  GapClusterer wire_clusters {0,1};
  GapClusterer grid_clusters {0,1};
  GapMatcher matcher {300000, 1};

  std::deque<uint64_t> pulse_times;

private:
  uint64_t previous_time_{0};

};

}