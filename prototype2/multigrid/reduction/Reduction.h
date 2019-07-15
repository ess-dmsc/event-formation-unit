/** Copyright (C) 2016-2018 European Spallation Source */

#pragma once
#include <common/clustering/GapClusterer.h>
#include <common/clustering/GapMatcher.h>
//#include <common/reduction/MgAnalyzer.h>
#include <multigrid/reduction/EventAnalysis.h>
#include <logical_geometry/ESSGeometry.h>
#include <common/reduction/ChronoMerger.h>

namespace Multigrid {

// Just greater than shortest pulse period of 266662 ticks
// Will have to be adjusted for other experimental setups
static constexpr uint64_t sequoia_maximum_latency {300000};

struct EventProcessingStats {
  size_t invalid_modules{0};
  size_t invalid_planes{0};
  size_t time_seq_errors{0};
  size_t wire_clusters{0};
  size_t grid_clusters{0};
  size_t events_total{0};
  size_t events_multiplicity_rejects{0};
  size_t hits_used{0};
  size_t events_bad{0};
  size_t events_geometry_err{0};

  void clear();
  EventProcessingStats& operator +=(const EventProcessingStats& other);
  std::string debug(std::string prepend) const;
};

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

  EventAnalyzer analyzer;

  ESSGeometry geometry;

private:
  uint64_t previous_time_{0};

};

void from_json(const nlohmann::json &j, ModulePipeline &g);


class Reduction {
public:
  Reduction();
  void ingest(HitVector &hits);
  void ingest(const Hit& hit);
  void process_queues(bool flush);
  std::string config(const std::string& prepend) const;
  std::string status(const std::string& prepend, bool verbose) const;

  uint32_t max_x() const;
  uint32_t max_y() const;
  uint32_t max_z() const;

  std::vector<ModulePipeline> pipelines;
  EventProcessingStats stats;

  ChronoMerger merger{sequoia_maximum_latency, 2};

  std::list<NeutronEvent> out_queue;

private:

};

void from_json(const nlohmann::json &j, Reduction &g);


}