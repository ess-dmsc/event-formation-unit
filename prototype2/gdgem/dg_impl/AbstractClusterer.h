#pragma once

#include <list>
#include <gdgem/dg_impl/HitContainer.h>
#include <gdgem/nmx/EventNMX.h>

using ClusterList = std::list<PlaneNMX>;

class AbstractClusterer {
public:
  AbstractClusterer() {}
  virtual ~AbstractClusterer() {}

  virtual void cluster(const HitContainer &hits) = 0;

  size_t stats_cluster_count{0};
  ClusterList clusters;
};
