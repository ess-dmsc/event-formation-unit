#pragma once

#include <list>
#include <gdgem/clustering/HitContainer.h>
#include <gdgem/nmx/Event.h>

// TODO: pick a faster data structure
using ClusterList = std::list<Cluster>;

class AbstractClusterer {
public:
  AbstractClusterer() {}
  virtual ~AbstractClusterer() {}

  virtual void cluster(const HitContainer &hits) = 0;

  bool empty() const
  {
    return clusters.empty();
  }

  uint8_t plane{0};
  size_t stats_cluster_count{0};
  ClusterList clusters;
};
