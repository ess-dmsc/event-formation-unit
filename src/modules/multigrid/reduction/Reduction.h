/** Copyright (C) 2016-2018 European Spallation Source */

#pragma once
#include <common/reduction/ChronoMerger.h>
#include <multigrid/reduction/ModulePipeline.h>

namespace Multigrid {

class Reduction {
public:
  Reduction() = default;
  void ingest(HitVector &hits);
  void ingest(const Hit &hit);
  void process_queues(bool flush);
  std::string config(const std::string &prepend) const;
  std::string status(const std::string &prepend, bool verbose) const;

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

} // namespace Multigrid