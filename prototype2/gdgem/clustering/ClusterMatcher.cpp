#include <gdgem/clustering/ClusterMatcher.h>

#include <cmath>
#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

ClusterMatcher::ClusterMatcher(double dPlane) : pdPlane(dPlane) {
}

bool ClusterMatcher::ready(const ClusterList& clusters) const
{
  return ((clusters.size() > 2) && ready(clusters.front().time_end, clusters));
}

bool ClusterMatcher::ready(double time, const ClusterList& clusters) const
{
  return ((clusters.size() > 2) &&
      (clusters.back().time_start - time) > (pdPlane*3));
}

bool ClusterMatcher::can_discard(double time, const Event& event) const
{
  return (!event.empty() && (time - event.time_end()) > (pdPlane*5));
}

double ClusterMatcher::delta1(const Event& event, const Cluster& cluster) const
{
  return std::abs(event.time_end() - cluster.time_end);
}

bool ClusterMatcher::belongs1(const Event& event, const Cluster& cluster) const
{
  return (delta1(event, cluster) <= pdPlane);
}

void ClusterMatcher::match_end(ClusterList& x_clusters,
                                  ClusterList& y_clusters, bool force) {

  if (!force && (!ready(x_clusters) || !ready(y_clusters)))
    return;

  Event evt;
  for (auto nx = x_clusters.begin(); nx != x_clusters.end();) {
    if (!force && !ready(nx->time_end, x_clusters))
      break;

    if (can_discard(nx->time_start, evt))
      break;

    // if matched event exists but new cluster does not belong
    // stash it, increment and reset
    if (!evt.empty() && !belongs1(evt, *nx)) {
      matched_clusters.emplace_back(std::move(evt));
      stats_cluster_count++;
      evt = Event();
    }

    // it was either emptied or still overlapping
    evt.merge(*nx, 0);
    x_clusters.erase(nx++);

    for (auto ny = y_clusters.begin(); ny != y_clusters.end();) {
      if (!force && !ready(ny->time_end, y_clusters))
        break;

      if (can_discard(ny->time_start, evt))
        break;

      if (belongs1(evt, *ny)) {
        evt.merge(*ny, 1);
        y_clusters.erase(ny++);
      } else
        ny++;
    }
  }

  // If anything is left, stash it
  if (!evt.empty()) {
    matched_clusters.emplace_back(std::move(evt));
    stats_cluster_count++;
  }

}

void ClusterMatcher::match_overlap(ClusterList& x_clusters,
                                      ClusterList& y_clusters) {
  double max_gap = 200;
  double overlap_thresh = 0.7;
  Event evt;

  for (auto nx = x_clusters.begin(); nx != x_clusters.end();) {
    // if matched cluster exists and we no longer overlap
    // stash it, increment and reset
    if (!evt.empty() && !evt.time_overlap_thresh(*nx, overlap_thresh)) {
      matched_clusters.emplace_back(std::move(evt));
      stats_cluster_count++;
      evt = Event();
    }

    // it was either emptied or still overlapping
    evt.merge(*nx, 0);
    x_clusters.erase(nx++);

    for (auto ny = y_clusters.begin(); ny != y_clusters.end();) {
      if ((ny->time_start - evt.time_end()) > max_gap)
        break;
      else if (evt.time_overlap_thresh(*ny, overlap_thresh)) {
        evt.merge(*ny, 1);
        y_clusters.erase(ny++);
      } else
        ny++;
    }
  }
}
