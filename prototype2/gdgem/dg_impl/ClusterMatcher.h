#pragma once

#include <gdgem/dg_impl/NMXClusterer.h>

class NMXClusterMatcher {
public:
  NMXClusterMatcher(double dPlane);
  void match_end(NMXClusterer& x, NMXClusterer& y, bool force);
  void match_overlap(NMXClusterer& x, NMXClusterer& y);

  size_t stats_cluster_count {0};
  std::list<EventNMX> matched_clusters;

private:
  double pdPlane {0};
};
