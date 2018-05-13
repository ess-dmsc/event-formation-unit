#include <gdgem/clustering/Clusterer1.h>
#include <algorithm>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

Clusterer1::Clusterer1(double maxTimeGap, uint16_t maxStripGap, size_t minClusterSize)
    : AbstractClusterer(), pMaxTimeGap(maxTimeGap), pMaxStripGap(maxStripGap), pMinClusterSize(minClusterSize) {
}

void Clusterer1::cluster(const HitContainer &hits) {
  cluster_by_time(hits);
}

void Clusterer1::cluster_by_time(const HitContainer &hits) {
  HitContainer cluster;

  for (const auto &hit : hits) {
    // Stash cluster if time gap to next hit is too large
    if (!cluster.empty() &&
        (hit.time - cluster.back().time) > pMaxTimeGap) {
      cluster_by_strip(cluster);
      cluster.clear();
    }

    // Insert in either case
    cluster.emplace_back(hit);
  }

  if (cluster.size())
    cluster_by_strip(cluster);
}

//====================================================================================================================
void Clusterer1::cluster_by_strip(HitContainer &hits) {
  Cluster cluster;

  std::sort(hits.begin(), hits.end(),
            [](const Eventlet &e1, const Eventlet &e2) {
              return e1.strip < e2.strip;
            });

  for (auto &hit : hits) {
    // Stash cluster if strip gap to next hit is too large
    if (!cluster.empty() &&
        (hit.strip - cluster.strip_end) > (pMaxStripGap + 1)) {
      stash_cluster(cluster);
      cluster = Cluster();
    }

    // insert in either case
    cluster.insert_eventlet(hit);
  }

  // At the end of the clustering, attempt to stash any leftovers
  stash_cluster(cluster);
}
//====================================================================================================================
void Clusterer1::stash_cluster(Cluster &cluster) {

  // TODO: Decide if filters go here

  if (cluster.entries.size() < pMinClusterSize)
    return;

  // TODO: time span filter?

  DTRACE(DEB, "******** VALID ********\n");
  clusters.emplace_back(std::move(cluster));
  stats_cluster_count++;
}
