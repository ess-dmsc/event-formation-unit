/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/reduction/ModulePipeline.h>
#include <multigrid/geometry/PlaneMappings.h>

#include <common/Trace.h>
#include <sstream>
#include <fmt/format.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

namespace Multigrid {

ModulePipeline::ModulePipeline() {
  matcher.set_minimum_time_gap(1);
}

void ModulePipeline::ingest(const Hit &hit) {
  if (previous_time_ > hit.time) {
    stats.time_seq_errors++;
    process_events(true);
  }
  previous_time_ = hit.time;

  auto plane = plane_in_module(hit.plane);

  if (plane == wire_plane) {
    wire_clusterer.insert(hit);
  } else if (plane == grid_plane) {
    grid_clusterer.insert(hit);
  } else {
    stats.invalid_planes++;
  }
}

void ModulePipeline::process_events(bool flush) {
  if (flush) {
    wire_clusterer.flush();
    grid_clusterer.flush();
  }
  stats.wire_clusters = wire_clusterer.stats_cluster_count;
  stats.grid_clusters = grid_clusterer.stats_cluster_count;
  if (!wire_clusterer.clusters.empty())
    matcher.insert(wire_clusterer.clusters.front().plane(), wire_clusterer.clusters);
  if (!grid_clusterer.clusters.empty())
    matcher.insert(grid_clusterer.clusters.front().plane(), grid_clusterer.clusters);
  matcher.match(flush);

  for (auto &event : matcher.matched_events) {

    stats.events_total++;

    if ((MGAnalyzer::WireCluster(event).hit_count() > max_wire_hits) ||
        (MGAnalyzer::GridCluster(event).hit_count() > max_grid_hits)) {
      stats.events_multiplicity_rejects++;
      continue;
    }

    auto neutron = analyzer.analyze(event);
    stats.hits_used = analyzer.stats_used_hits;

    if (!neutron.good) {
      stats.events_bad++;
      continue;
    }
    //            XTRACE(PROCESS, DEB, "Neutron: %s ", neutron.to_string().c_str());
    uint32_t pixel = geometry.pixel3D(
        neutron.x.center_rounded(),
        neutron.y.center_rounded(),
        neutron.z.center_rounded()
    );

    if (pixel == 0) {
      XTRACE(PROCESS, DEB, "Event geometry error for %s\n      %s",
             neutron.to_string().c_str(), event.to_string("      ", true).c_str());
      stats.events_geometry_err++;
      continue;
    }
//            XTRACE(PROCESS, DEB, "Event good");

    out_queue.push_back({neutron.time, pixel});
  }
  matcher.matched_events.clear();
}

std::string ModulePipeline::config(const std::string& prepend) const {
  std::stringstream ss;
  ss << prepend << "Wire clusterer:\n" + wire_clusterer.config(prepend + "  ");
  ss << prepend << "Grid clusterer:\n" + grid_clusterer.config(prepend + "  ");
  ss << prepend << "Matcher:\n" + matcher.config(prepend + "  ");
  ss << prepend << "max_wire_hits = " << max_wire_hits << "\n";
  ss << prepend << "max_grid_hits = " << max_grid_hits << "\n";
  ss << prepend << analyzer.debug(prepend + "  ");
  return ss.str();
}

std::string ModulePipeline::status(const std::string& prepend, bool verbose) const {
  std::stringstream ss;
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
  ss << prepend << "Previous time: " << previous_time_ << "\n";
  ss << prepend << "Matcher:\n" + matcher.status(prepend + "  ", verbose);
  ss << prepend << "Wire clusterer:\n" + wire_clusterer.status(prepend + "  ", verbose);
  ss << prepend << "Grid clusterer:\n" + grid_clusterer.status(prepend + "  ", verbose);
  return ss.str();
}

}