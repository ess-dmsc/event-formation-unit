/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/reduction/Reduction.h>
#include <multigrid/AbstractBuilder.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

namespace Multigrid {

void EventProcessingStats::clear() {
  invalid_planes = 0;
  time_seq_errors = 0;
  wire_clusters = 0;
  grid_clusters = 0;
  events_total = 0;
  events_multiplicity_rejects = 0;
  hits_used = 0;
  events_bad = 0;
  events_geometry_err = 0;
}

EventProcessingStats &EventProcessingStats::operator+=(const EventProcessingStats &other) {
  invalid_planes += other.invalid_planes;
  time_seq_errors += other.time_seq_errors;
  wire_clusters += other.wire_clusters;
  grid_clusters += other.grid_clusters;
  events_total += other.events_total;
  events_multiplicity_rejects += other.events_multiplicity_rejects;
  hits_used += other.hits_used;
  events_bad += other.events_bad;
  events_geometry_err += other.events_geometry_err;
  return *this;
}

ModulePipeline::ModulePipeline() {
  matcher.set_minimum_time_gap(1);
}

void ModulePipeline::ingest(const Hit &hit) {
  if (previous_time_ > hit.time) {
    stats.time_seq_errors++;
    process_events(true);
  }
  previous_time_ = hit.time;

  auto plane = hit.plane % 2;

  if (plane == AbstractBuilder::wire_plane) {
    wire_clusters.insert(hit);
  } else if (plane == AbstractBuilder::grid_plane) {
    grid_clusters.insert(hit);
  } else {
    stats.invalid_planes++;
  }
}

void ModulePipeline::process_events(bool flush) {
  if (flush) {
    wire_clusters.flush();
    grid_clusters.flush();
  }
  stats.wire_clusters = wire_clusters.stats_cluster_count;
  stats.grid_clusters = grid_clusters.stats_cluster_count;
  if (!wire_clusters.clusters.empty())
    matcher.insert(wire_clusters.clusters.front().plane(), wire_clusters.clusters);
  if (!grid_clusters.clusters.empty())
    matcher.insert(grid_clusters.clusters.front().plane(), grid_clusters.clusters);
  matcher.match(flush);

  for (auto &event : matcher.matched_events) {

    stats.events_total++;

    if ((event.cluster1.hit_count() > max_wire_hits) ||
        (event.cluster2.hit_count() > max_grid_hits)) {
      stats.events_multiplicity_rejects++;
      continue;
    }

    auto neutron = analyzer.analyze(event);
    stats.hits_used = analyzer.stats_used_hits;

    if (!neutron.good) {
      stats.events_bad++;
      continue;
    }
    //            XTRACE(PROCESS, DEB, "Neutron: %s ", neutron.debug().c_str());
    uint32_t pixel = geometry.pixel3D(
        neutron.x.center_rounded(),
        neutron.y.center_rounded(),
        neutron.z.center_rounded()
    );

    if (pixel == 0) {
      XTRACE(PROCESS, DEB, "Event geometry error");
      stats.events_geometry_err++;
      continue;
    }
//            XTRACE(PROCESS, DEB, "Event good");

    out_queue.push_back({neutron.time, pixel});
  }
  matcher.matched_events.clear();
}

std::string ModulePipeline::debug(std::string prepend) const {
  std::stringstream ss;
  ss << prepend << "  Event position using weighted average: "
     << (analyzer.weighted() ? "YES" : "no") << "\n";
  ss << prepend << "  max_wire_hits = " << max_wire_hits << "\n";
  ss << prepend << "  max_grid_hits = " << max_grid_hits << "\n";
  ss << prepend << "  geometry_x = " << geometry.nx() << "\n";
  ss << prepend << "  geometry_y = " << geometry.ny() << "\n";
  ss << prepend << "  geometry_z = " << geometry.nz() << "\n";
  return ss.str();
}

Reduction::Reduction() {

}

void Reduction::ingest(HitVector &hits) {
  for (const auto &h : hits) {
    ingest(h);
  }
  hits.clear();
}

void Reduction::ingest(const Hit &h) {
  if (h.plane == Hit::PulsePlane) {
    merger.insert(0, {h.time, 0});
//  } else if (h.plane == AbstractBuilder::wire_plane) {
//    pipeline.ingest(h);
//  } else if (h.plane == AbstractBuilder::grid_plane) {
//    pipeline.ingest(h);
  } else {
    auto module = h.plane / 2;
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
    for (const auto &event : p.out_queue)
      merger.insert(i + 1, event);
    p.out_queue.clear();
  }

  merger.sort();

  while (merger.ready())
    out_queue.push_back(merger.pop_earliest());

  if (flush) {
    while (!merger.empty())
      out_queue.push_back(merger.pop_earliest());
  }
}

}