#pragma once

#include <gdgem/dg_impl/NMXCluster.h>

struct CommonClusterNMX {
  int sizeX;
  int sizeY;
  int adcX;
  int adcY;
  double timeX;
  double timeY;
  double deltaPlane;
};

class NMXClusterMatcher {
public:
  NMXClusterMatcher(double dPlane);
  void match(std::vector<ClusterNMX>& x, std::vector<ClusterNMX>& y);

  size_t stats_cluster_count {0};
  ClusterVector matched_clusters;

private:
  double pdPlane {0};
};
