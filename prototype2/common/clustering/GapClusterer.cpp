/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <common/clustering/GapClusterer.h>
#include <algorithm>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

GapClusterer::GapClusterer(uint64_t TimeGap, uint16_t CoordGap, size_t ClusterSize)
    : AbstractClusterer(), MaxTimeGap(TimeGap), MaxCoordGap(CoordGap), MinClusterSize(ClusterSize) {
}

void GapClusterer::cluster(const HitContainer &hits) {
  cluster_by_time(hits);
}

void GapClusterer::flush() {
  cluster_by_coordinate(CurrentTimeCluster);
  CurrentTimeCluster.clear();
}

void GapClusterer::cluster_by_time(const HitContainer &hits) {
  for (const auto &hit : hits) {
    // Stash cluster if time gap to next hit is too large
    if (!CurrentTimeCluster.empty() &&
        (hit.time - CurrentTimeCluster.back().time) > MaxTimeGap) {
      flush();
    }

    // Insert in either case
    CurrentTimeCluster.emplace_back(hit);
  }
}

//====================================================================================================================
void GapClusterer::cluster_by_coordinate(HitContainer &hits) {
  Cluster cluster;

  std::sort(hits.begin(), hits.end(),
            [](const Hit &e1, const Hit &e2) {
              return e1.coordinate < e2.coordinate;
            });

  for (auto &hit : hits) {
    // Stash cluster if coordinate gap to next hit is too large
    if (!cluster.empty() &&
        (hit.coordinate - cluster.coord_end) > MaxCoordGap) {
      stash_cluster(cluster);
      cluster = Cluster();
    }

    // insert in either case
    cluster.insert_hit(hit);
  }

  // At the end of the clustering, attempt to stash any leftovers
  stash_cluster(cluster);
}
//====================================================================================================================
void GapClusterer::stash_cluster(Cluster &cluster) {
  if (cluster.entries.size() < MinClusterSize)
    return;

  DTRACE(DEB, "******** VALID ********");
  clusters.emplace_back(std::move(cluster));
  stats_cluster_count++;
}
