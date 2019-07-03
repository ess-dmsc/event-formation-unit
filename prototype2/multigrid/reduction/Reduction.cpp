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

void Reduction::ingest(const Hit& h)
{
  // We must do this because of patterns observed in BuilderReadoutTest
  if (previous_time_ > h.time) {
    stats_time_seq_errors++;
    perform_clustering(true);
  }
  previous_time_ = h.time;

  if (h.plane == Hit::PulsePlane) {
    pulse_times.push_back(h);
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
  matcher.insert_pulses(pulse_times);
  matcher.match(flush);
}

}