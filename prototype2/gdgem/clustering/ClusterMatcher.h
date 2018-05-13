#pragma once

#include <memory>
#include <gdgem/clustering/AbstractClusterer.h>

class ClusterMatcher {
public:
  ClusterMatcher(double maxDeltaTime);

  void merge(ClusterList& c);

  void match_end(bool force);

  // TODO: match in other ways -- mass, overlap?

  size_t stats_cluster_count {0};

  ClusterList unmatched_clusters;
  std::list<Event> matched_clusters;

private:
  double pMaxDeltaTime {0};

  bool ready(double time) const;

  double delta_end(const Event& event, const Cluster& cluster) const;
  bool belongs_end(const Event& event, const Cluster& cluster) const;
};
