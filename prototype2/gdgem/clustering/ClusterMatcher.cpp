#include <gdgem/clustering/ClusterMatcher.h>

#include <cmath>
#include <algorithm>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

ClusterMatcher::ClusterMatcher(double maxDeltaTime) : pMaxDeltaTime(maxDeltaTime) {
}

bool ClusterMatcher::ready_to_be_matched(double time) const {
  // TODO Parametrize threshold
  return ((unmatched_clusters.size() > 2) &&
      (std::min(latest_x, latest_y) - time) > (pMaxDeltaTime * 3));
}

double ClusterMatcher::delta_end(const Event &event, const Cluster &cluster) const {
  return std::abs(event.time_end() - cluster.time_end);
}

bool ClusterMatcher::belongs_end(const Event &event, const Cluster &cluster) const {
  return (delta_end(event, cluster) <= pMaxDeltaTime);
}

// brackets
void ClusterMatcher::merge(uint8_t plane, ClusterList &c) {
  if (c.empty())
    return;
  if (plane == 1)
    latest_y = std::max(latest_y, c.back().time_start);
  else // plane = 0
    latest_x = std::max(latest_x, c.back().time_start);
  unmatched_clusters.splice(unmatched_clusters.end(), c);
}

void ClusterMatcher::match_end(bool force) {
  // Perhaps it is already sorted?
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
