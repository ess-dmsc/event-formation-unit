#include <gdgem/clustering/ClusterMatcher.h>

#include <cmath>
#include <algorithm>

#include <common/Trace.h>
#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

ClusterMatcher::ClusterMatcher(double dPlane) : pdPlane(dPlane) {
}

bool ClusterMatcher::ready(double time) const
{
  return ((unmatched_clusters.size() > 2) &&
      (unmatched_clusters.back().time_start - time) > (pdPlane*3));
}

double ClusterMatcher::delta_end(const Event& event, const Cluster& cluster) const
{
  return std::abs(event.time_end() - cluster.time_end);
}

bool ClusterMatcher::belongs_end(const Event& event, const Cluster& cluster) const
{
  return (delta_end(event, cluster) <= pdPlane);
}

void ClusterMatcher::merge(ClusterList& c) {
  unmatched_clusters.splice(unmatched_clusters.end(), c);
}

void ClusterMatcher::match_end(bool force) {

  unmatched_clusters.sort([](const Cluster &c1, const Cluster &c2) {
    return c1.time_end < c2.time_end;
  });

  Event evt;

  while (!unmatched_clusters.empty()) {

    auto n = unmatched_clusters.begin();

    if (!force && !ready(n->time_end))
      break;

    if (!evt.empty() && !belongs_end(evt, *n)) {
      matched_clusters.emplace_back(std::move(evt));
      stats_cluster_count++;
      evt = Event();
    }

    evt.merge(*n);
    unmatched_clusters.pop_front();
  }

  // If anything is left, stash it
  if (!evt.empty()) {
    matched_clusters.emplace_back(std::move(evt));
    stats_cluster_count++;
  }
}
