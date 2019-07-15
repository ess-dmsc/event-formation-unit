/** Copyright (C) 2016-2018 European Spallation Source */

#pragma once
#include <multigrid/reduction/EventProcessingStats.h>
#include <common/clustering/GapClusterer.h>
#include <common/clustering/GapMatcher.h>
#include <common/reduction/MgAnalyzer.h>
#include <common/reduction/NeutronEvent.h>
#include <logical_geometry/ESSGeometry.h>

namespace Multigrid {

// Just greater than shortest pulse period of 266662 ticks
// Will have to be adjusted for other experimental setups
static constexpr uint64_t sequoia_maximum_latency {300000};

class ModulePipeline {
public:
  ModulePipeline();
  void ingest(const Hit& hit);
  void process_events(bool flush);
  std::string config(const std::string& prepend) const;
  std::string status(const std::string& prepend, bool verbose) const;

  std::list<NeutronEvent> out_queue;

  EventProcessingStats stats;

//private:

// \todo use GapClusterer2D for wires
  GapClusterer wire_clusterer{0, 1};
  GapClusterer grid_clusterer{0, 1};

  GapMatcher matcher{sequoia_maximum_latency, 0, 1};

  size_t max_wire_hits {12};
  size_t max_grid_hits {12};

  MGAnalyzer analyzer;

  ESSGeometry geometry;

private:
  uint64_t previous_time_{0};

};

void from_json(const nlohmann::json &j, ModulePipeline &g);


}