/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/reduction/Reduction.h>
#include <multigrid/geometry/PlaneMappings.h>

#include <common/debug/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

namespace Multigrid {

void Reduction::ingest(HitVector &hits) {
  for (const auto &h : hits) {
    ingest(h);
  }
  hits.clear();
}

void Reduction::ingest(const Hit &h) {
  if (h.plane == Hit::PulsePlane) {
    merger.insert(pipelines.size(), {h.time, 0});
//  } else if (h.plane == AbstractBuilder::wire_plane) {
//    pipeline.ingest(h);
//  } else if (h.plane == AbstractBuilder::grid_plane) {
//    pipeline.ingest(h);
  } else {
    auto module = module_from_plane(h.plane);
    pipelines[module].ingest(h);
//    stats.invalid_planes++;
  }
}

void Reduction::process_queues(bool flush) {
  stats.clear();
  for (size_t i = 0; i < pipelines.size(); ++i) {
    auto &p = pipelines[i];
    p.process_events(flush);
    stats += p.stats;
    merger.insert(i, p.out_queue);
  }

  // \todo refactor: remove this hack!
  if (pipelines.size() == 9) {
    merger.sync_up(0, 1);
    merger.sync_up(0, 2);
    merger.sync_up(1, 2);

    merger.sync_up(3, 4);
    merger.sync_up(3, 5);
    merger.sync_up(4, 5);

    merger.sync_up(6, 7);
    merger.sync_up(6, 8);
    merger.sync_up(7, 8);
  }

  merger.sort();

  while (merger.ready())
    out_queue.push_back(merger.pop_earliest());

  if (flush) {
    while (!merger.empty())
      out_queue.push_back(merger.pop_earliest());
  }
}

uint32_t Reduction::max_x() const {
  uint32_t ret{0};
  for (const auto &b : pipelines)
    ret = std::max(ret, b.analyzer.geometry().x_offset
        + b.analyzer.geometry().x_range());
  return ret;
}

uint32_t Reduction::max_y() const {
  uint32_t ret{0};
  for (const auto &b : pipelines)
    ret = std::max(ret, b.analyzer.geometry().y_offset
        + b.analyzer.geometry().y_range());
  return ret;
}

uint32_t Reduction::max_z() const {
  uint32_t ret{0};
  for (const auto &b : pipelines)
    ret = std::max(ret, b.analyzer.geometry().z_offset
        + b.analyzer.geometry().z_range());
  return ret;
}

std::string Reduction::config(const std::string &prepend) const {
  std::stringstream ss;

  ss << "--== PIPELINE CONFIG ==--\n";

  for (size_t i = 0; i < pipelines.size(); ++i) {
    ss << prepend + "  Module[" << i << "]\n";
    ss << pipelines[i].config(prepend + "  ");
    ss << "\n";
  }

  if (!pipelines.empty()) {
    const auto &p = pipelines.back();
    ss << prepend
       << fmt::format("  Logical geometry:  {}\n",
                      p.geometry.to_string());
  }

  ss << prepend << "Merger:\n" << merger.debug(prepend + "  ", false);

  return ss.str();
}

std::string Reduction::status(const std::string &prepend, bool verbose) const {
  std::stringstream ss;

  ss << prepend << "--== PIPELINE STATUS ==--\n";
  ss << prepend << "Stats:\n" << stats.debug(prepend + "  ");

  if (!out_queue.empty()) {
    ss << prepend << fmt::format("Out queue [{}]\n", out_queue.size());
    if (verbose) {
      // \todo refactor
      for (const auto &e : out_queue) {
        ss << prepend << "  " << e.to_string() << "\n";
      }
    }
  }

  ss << prepend << "Merger:\n" << merger.debug(prepend + "  ", verbose);

  for (size_t i = 0; i < pipelines.size(); ++i) {
    ss << prepend + "Module[" << i << "]\n";
    ss << pipelines[i].status(prepend + "  ", verbose);
  }

  return ss.str();
}

void from_json(const nlohmann::json &j, Reduction &g) {
  uint64_t max_latency = j["maximum_latency"];
  size_t max_wire_multiplicity = j["max_wire_multiplicity"];
  size_t max_grid_multiplicity = j["max_grid_multiplicity"];

  size_t module_count = 0;
  for (const auto &jj : j["modules"]) {
    ModulePipeline pipeline;

    // \todo custom clusterer settings

    pipeline.matcher = GapMatcher(max_latency,
                                  2 * module_count,
                                  2 * module_count + 1);

    pipeline.max_wire_hits = max_wire_multiplicity;
    pipeline.max_grid_hits = max_grid_multiplicity;

    pipeline.analyzer.weighted(jj["analysis_weighted"]);
    pipeline.analyzer.set_geometry(jj["digital_geometry"]);

    g.pipelines.push_back(pipeline);
    module_count++;
  }

  ESSGeometry logical_geometry(g.max_x(), g.max_y(), g.max_z(), 1);

  for (auto &p : g.pipelines) {
    p.geometry = logical_geometry;
  }

  g.merger = ChronoMerger(max_latency, g.pipelines.size() + 1);
}

}