#pragma once

#include <list>
#include <gdgem/dg_impl/HitContainer.h>
#include <gdgem/nmx/EventNMX.h>

using ClusterList = std::list<PlaneNMX>;

class NMXClusterer {
public:
  NMXClusterer(double maxTimeGap, uint16_t maxStripGap, size_t minClusterSize);

  void cluster(const HitContainer &hits);

  bool ready() const;
  bool ready(double time) const;


  size_t stats_cluster_count{0};
  ClusterList clusters;

private:
  double pMaxTimeGap;
  uint16_t pMaxStripGap;
  size_t pMinClusterSize;

  void cluster_by_time(const HitContainer &oldHits);
  void cluster_by_strip(HitContainer &cluster);
  void stash_cluster(PlaneNMX &plane);
};
