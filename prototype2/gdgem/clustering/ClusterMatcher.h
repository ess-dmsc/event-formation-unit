#pragma once

#include <memory>
#include <gdgem/clustering/AbstractClusterer.h>

class ClusterMatcher {
public:
  ClusterMatcher(double dPlane);
  void match_end(ClusterList& x,
                 ClusterList& y, bool force);
  void match_overlap(ClusterList& x,
                     ClusterList& y);

  size_t stats_cluster_count {0};
  std::list<Event> matched_clusters;

private:
  double pdPlane {0};

  bool ready(const ClusterList&) const;
  bool ready(double time, const ClusterList&) const;
  bool can_discard(double time, const Event& event) const;

  double delta1(const Event& event, const PlaneNMX& cluster) const;
  bool belongs1(const Event& event, const PlaneNMX& cluster) const;
};
