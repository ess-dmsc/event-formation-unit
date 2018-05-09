#pragma once

#include <gdgem/dg_impl/NMXCluster.h>

class NMXClusterMatcher {
public:
  NMXClusterMatcher(double dPlane);
  void match_end(ClusterVector& x, ClusterVector& y);
  void match_overlap(ClusterVector& x, ClusterVector& y);

  size_t stats_cluster_count {0};
  std::list<EventNMX> matched_clusters;

private:
  double pdPlane {0};
};
