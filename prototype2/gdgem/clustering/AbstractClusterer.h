#pragma once

#include <list>
#include <gdgem/clustering/HitContainer.h>
#include <gdgem/nmx/Event.h>

using ClusterList = std::list<Cluster>;

class AbstractClusterer {
public:
  AbstractClusterer() {}
  virtual ~AbstractClusterer() {}

  virtual void cluster(const HitContainer &hits) = 0;

  size_t stats_cluster_count{0};
  ClusterList clusters;
};
