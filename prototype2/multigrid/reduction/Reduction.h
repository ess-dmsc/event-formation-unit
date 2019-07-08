/** Copyright (C) 2016-2018 European Spallation Source */

#pragma once
#include <common/clustering/GapClusterer.h>
#include <common/clustering/GapMatcher.h>
//#include <common/reduction/MgAnalyzer.h>
#include <multigrid/reduction/EventAnalysis.h>
#include <logical_geometry/ESSGeometry.h>
#include <common/reduction/ChronoMerger.h>

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
  size_t stats_events_total{0};
  size_t stats_events_multiplicity_rejects{0};
  size_t stats_hits_used{0};
  size_t stats_events_bad{0};
  size_t stats_events_geometry_err{0};

  size_t max_wire_hits {12};
  size_t max_grid_hits {12};

  EventAnalyzer analyzer;

  ESSGeometry geometry;

  ChronoMerger merger{300000, 2};

  std::list<NeutronEvent> out_queue;

private:
  GapClusterer wire_clusters{0, 1};
  GapClusterer grid_clusters{0, 1};

  // Just greater than shortest pulse period of 266662 ticks
  // Will have to be adjusted for other experimental setups
  GapMatcher matcher{300000, 0, 1};

  uint64_t previous_time_{0};
};

}