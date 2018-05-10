#include <gdgem/dg_impl/NMXClusterer.h>
#include <algorithm>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

NMXClusterer::NMXClusterer(SRSTime time, size_t minClusterSize, double deltaTimeHits,
                           uint16_t deltaStripHits) :
    pTime(time), pMinClusterSize(minClusterSize), pDeltaTimeHits(deltaTimeHits),
    pMaximumStripSeparation(deltaStripHits)
{
}

void NMXClusterer::cluster(const HitContainer &hits) {
  cluster_by_time(hits);
}

void NMXClusterer::cluster_by_time(const HitContainer &hits) {

  auto pre = clusters.size();

  HitContainer cluster;
  double maxDeltaTime = 0;
  size_t stripCount = 0;
  double time1 = 0, time2 = 0;
  uint16_t adc1 = 0;
  uint16_t strip1 = 0;

  for (const auto &itHits : hits) {
    time2 = time1;

    time1 = itHits.time;
    strip1 = itHits.strip;
    adc1 = itHits.adc;

    if (time1 - time2 <= pDeltaTimeHits && stripCount > 0
        && maxDeltaTime < (time1 - time2)) {
      maxDeltaTime = (time1 - time2);
    }

    if (time1 - time2 > pDeltaTimeHits && stripCount > 0) {
      cluster_by_strip(cluster);
      cluster.clear();
      maxDeltaTime = 0;
    }
    cluster.emplace_back(Eventlet());
    auto &e = cluster[cluster.size()-1];
    e.strip = strip1;
    e.time = time1;
    e.adc = adc1;
    stripCount++;
  }

  if (stripCount > 0)
    cluster_by_strip(cluster);

  stats_cluster_count += (clusters.size() - pre);
}

//====================================================================================================================
void NMXClusterer::cluster_by_strip(HitContainer &hits) {
  PlaneNMX cluster;

  std::sort(hits.begin(), hits.end(),
            [](const Eventlet &e1, const Eventlet &e2) {
              return e1.strip < e2.strip;
            });

  for (auto &hit : hits) {
    // If empty cluster, just add and move on
    if (!cluster.entries.size()) {
      cluster.insert_eventlet(hit);
      continue;
    }

    // Stash cluster if strip gap to next hit is too large
    // filtering is done elsewhere
    auto strip_gap = std::abs(hit.strip - cluster.strip_end);
    if (strip_gap > (pMaximumStripSeparation + 1))
    {
      // Attempt to stash
      stash_cluster(cluster);

      // Reset and move on
      cluster = PlaneNMX();
    }

    // insert in either case
    cluster.insert_eventlet(hit);
  }

  // At the end of the clustering, check again if there is a last valid cluster
  if (cluster.entries.size() >= pMinClusterSize) {
    stash_cluster(cluster);
  }
}
//====================================================================================================================
void NMXClusterer::stash_cluster(PlaneNMX& cluster) {

  // Some filtering can happen here
  if (cluster.entries.size() < pMinClusterSize)
    return;

  // pDeltaTimeSpan ?

  DTRACE(DEB, "******** VALID ********\n");
  clusters.emplace_back(std::move(cluster));
}

bool NMXClusterer::ready() const
{
  return ((clusters.size() > 2) &&
      (clusters.back().time_start - clusters.front().time_end) > (pDeltaTimeHits*3));
}