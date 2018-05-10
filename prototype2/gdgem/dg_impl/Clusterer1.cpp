#include <gdgem/dg_impl/Clusterer1.h>
#include <algorithm>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

Clusterer1::Clusterer1(double maxTimeGap, uint16_t maxStripGap, size_t minClusterSize)
    : AbstractClusterer()
    , pMaxTimeGap(maxTimeGap)
    , pMaxStripGap(maxStripGap)
    , pMinClusterSize(minClusterSize)
{
}

void Clusterer1::cluster(const HitContainer &hits) {
  cluster_by_time(hits);
}

void Clusterer1::cluster_by_time(const HitContainer &hits) {
  HitContainer cluster;

  for (const auto &hit : hits) {
    // If empty cluster, just add and move on
    if (cluster.empty()) {
      cluster.emplace_back(hit);
      continue;
    }

    // Stash cluster if time gap to next hit is too large
    auto time_gap = hit.time - cluster.back().time;
    if (time_gap > pMaxTimeGap) {
      cluster_by_strip(cluster);
      cluster.clear();
    }

    // insert in either case
    cluster.emplace_back(hit);
  }

  if (cluster.size())
    cluster_by_strip(cluster);
}

//====================================================================================================================
void Clusterer1::cluster_by_strip(HitContainer &hits) {
  PlaneNMX cluster;

  std::sort(hits.begin(), hits.end(),
            [](const Eventlet &e1, const Eventlet &e2) {
              return e1.strip < e2.strip;
            });

  for (auto &hit : hits) {
    // If empty cluster, just add and move on
    if (cluster.entries.empty()) {
      cluster.insert_eventlet(hit);
      continue;
    }

    // Stash cluster if strip gap to next hit is too large
    // filtering is done elsewhere
    auto strip_gap = hit.strip - cluster.strip_end;
    if (strip_gap > (pMaxStripGap + 1))
    {
      // Attempt to stash
      stash_cluster(cluster);

      // Reset and move on
      cluster = PlaneNMX();
    }

    // insert in either case
    cluster.insert_eventlet(hit);
  }

  // At the end of the clustering, attempt to stash any leftovers
  stash_cluster(cluster);
}
//====================================================================================================================
void Clusterer1::stash_cluster(PlaneNMX& cluster) {

  // Some filtering can happen here
  if (cluster.entries.size() < pMinClusterSize)
    return;

  // pDeltaTimeSpan ?

  DTRACE(DEB, "******** VALID ********\n");
  clusters.emplace_back(std::move(cluster));
  stats_cluster_count ++;
}
