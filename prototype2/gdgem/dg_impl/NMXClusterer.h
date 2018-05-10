#pragma once

#include <list>
#include <gdgem/vmm2srs/SRSTime.h>
#include <gdgem/dg_impl/NMXCluster.h>
#include <gdgem/nmx/EventNMX.h>

using ClusterList = std::list<PlaneNMX>;

class NMXClusterer {
public:
  NMXClusterer(SRSTime time, size_t minClusterSize,
               double deltaTimeHits, uint16_t deltaStripHits);

  void cluster(const HitContainer &hits);

  bool ready() const;

  size_t stats_cluster_count{0};
  ClusterList clusters;

private:
  SRSTime pTime;
  size_t pMinClusterSize;
  double pDeltaTimeHits;
  uint16_t pMaximumStripSeparation;

  void cluster_by_time(const HitContainer &oldHits);
  void cluster_by_strip(HitContainer &cluster);
  void stash_cluster(PlaneNMX &plane);
};
