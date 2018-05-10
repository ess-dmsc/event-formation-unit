#pragma once

#include <gdgem/dg_impl/NMXClusterer.h>

class NMXClusterMatcher {
public:
  NMXClusterMatcher(double dPlane);
  void match_end(ClusterList& x, ClusterList& y);
  void match_overlap(ClusterList& x, ClusterList& y);

  size_t stats_cluster_count {0};
  std::list<EventNMX> matched_clusters;

private:
  double pdPlane {0};
};
