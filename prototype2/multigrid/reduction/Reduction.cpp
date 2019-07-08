/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/reduction/Reduction.h>
#include <multigrid/AbstractBuilder.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

namespace Multigrid {

Reduction::Reduction() {
  matcher.set_minimum_time_gap(1);
}

void Reduction::ingest(HitVector &hits) {
  for (const auto &h : hits) {
    ingest(h);
  }
  hits.clear();
}

void Reduction::ingest(const Hit &h) {
  // We must do this because of patterns observed in BuilderReadoutTest
  if (previous_time_ > h.time) {
    stats_time_seq_errors++;
    perform_clustering(true);
  }
  previous_time_ = h.time;

  if (h.plane == Hit::PulsePlane) {
    merger.insert(0, {h.time, 0});
    //pulse_times.push_back(h);
  } else if (h.plane == AbstractBuilder::wire_plane) {
    wire_clusters.insert(h);
  } else if (h.plane == AbstractBuilder::grid_plane) {
    grid_clusters.insert(h);
  } else {
    stats_invalid_planes++;
  }
}

void Reduction::perform_clustering(bool flush) {
  if (flush) {
    wire_clusters.flush();
    grid_clusters.flush();
  }
  stats_wire_clusters = wire_clusters.stats_cluster_count;
  stats_grid_clusters = grid_clusters.stats_cluster_count;
  matcher.insert(AbstractBuilder::wire_plane, wire_clusters.clusters);
  matcher.insert(AbstractBuilder::grid_plane, grid_clusters.clusters);
  //matcher.insert_pulses(pulse_times);
  matcher.match(flush);

  for (auto &event : matcher.matched_events) {

    stats_events_total++;

    if ((event.cluster1.hit_count() > max_wire_hits) ||
        (event.cluster2.hit_count() > max_grid_hits)) {
      stats_events_multiplicity_rejects++;
      continue;
    }

    auto neutron = analyzer.analyze(event);
    stats_hits_used = analyzer.stats_used_hits;

    if (!neutron.good) {
      stats_events_bad++;
      continue;
    }
    //            XTRACE(PROCESS, DEB, "Neutron: %s ", neutron.debug().c_str());
    uint32_t pixel = geometry.pixel3D(
        neutron.x.center_rounded(),
        neutron.y.center_rounded(),
        neutron.z.center_rounded()
    );

    if (pixel == 0) {
      XTRACE(PROCESS, DEB, "Event geom error");
      stats_events_geometry_err++;
      continue;
    }
//            XTRACE(PROCESS, DEB, "Event good");
    merger.insert(1, {neutron.time, pixel});
  }
  matcher.matched_events.clear();

  merger.sort();

  while (merger.ready())
    out_queue.push_back(merger.pop_earliest());

  if (flush) {
    while (!merger.empty())
      out_queue.push_back(merger.pop_earliest());
  }
}

}