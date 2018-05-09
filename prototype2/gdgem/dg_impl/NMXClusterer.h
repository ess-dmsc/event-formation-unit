#pragma once

#include <gdgem/vmm2srs/SRSTime.h>
#include <gdgem/dg_impl/NMXCluster.h>

class NMXClusterer {
public:
  NMXClusterer(SRSTime time, size_t minClusterSize,
               double deltaTimeHits, uint16_t deltaStripHits);

  void ClusterByTime(const HitContainer &oldHits);
  void ClusterByStrip(HitContainer &cluster);

  void stash_cluster(PlaneNMX& plane);

  bool ready() const;

  size_t stats_cluster_count{0};

  ClusterVector clusters;

private:
  SRSTime pTime;

  size_t pMinClusterSize;
  double pDeltaTimeHits;
  uint16_t pDeltaStripHits;
};
