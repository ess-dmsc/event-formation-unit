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

GapClusterer::GapClusterer(uint64_t maxTimeGap, uint16_t maxStripGap, size_t minClusterSize)
    : AbstractClusterer(), pMaxTimeGap(maxTimeGap), pMaxStripGap(maxStripGap), pMinClusterSize(minClusterSize) {
}

void GapClusterer::cluster(const HitContainer &hits) {
  cluster_by_time(hits);
}

void GapClusterer::cluster_by_time(const HitContainer &hits) {
  HitContainer cluster;

  for (const auto &hit : hits) {
    // Stash cluster if time gap to next hit is too large
    if (!cluster.empty() &&
        (hit.time - cluster.back().time) > pMaxTimeGap) {
      cluster_by_coordinate(cluster);
      cluster.clear();
    }

    // Insert in either case
    cluster.emplace_back(hit);
  }

  if (!cluster.empty()) {
    cluster_by_coordinate(cluster);
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
        (hit.coordinate - cluster.coord_end) > (pMaxStripGap + 1)) {
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
  if (cluster.entries.size() < pMinClusterSize)
    return;

  DTRACE(DEB, "******** VALID ********");
  clusters.emplace_back(std::move(cluster));
  stats_cluster_count++;
}
