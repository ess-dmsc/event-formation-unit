#include <gdgem/dg_impl/ClusterMatcher.h>

#include <cmath>
#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

NMXClusterMatcher::NMXClusterMatcher(double dPlane) : pdPlane(dPlane) {
}

void NMXClusterMatcher::match_end(NMXClusterer &x_clusters,
                                  NMXClusterer &y_clusters, bool force) {
  for (auto nx = x_clusters.clusters.begin(); nx != x_clusters.clusters.end();) {
    if (!force && !x_clusters.ready(nx->time_end))
      break;

    double tx = nx->time_end;

    double minDelta = 99999999;
    double deltaT = 0;
    ClusterList::iterator it_y = y_clusters.clusters.end();

    double ty = 0;

    for (auto ny = y_clusters.clusters.begin(); ny != y_clusters.clusters.end(); ++ny) {
      if (!force && !y_clusters.ready(nx->time_end))
        break;

      ty = ny->time_end;
      deltaT = std::abs(ty - tx);
      if (deltaT < minDelta && deltaT <= pdPlane) {
        minDelta = deltaT;
        it_y = ny;
      }
    }

    if (it_y != y_clusters.clusters.end()) {
      DTRACE(DEB, "\ttime x/time y: : %f/%f", tx, ty);

      y_clusters.clusters.erase(it_y++);
      x_clusters.clusters.erase(nx++);

      //m_clusterXY.emplace_back(std::move(theCommonCluster));
      // callback(theCommonCluster))
      stats_cluster_count++;
    } else
      nx++;
  }

//  x_clusters.clear();
//  y_clusters.clear();
}

void NMXClusterMatcher::match_overlap(NMXClusterer &x_clusters, NMXClusterer &y_clusters) {
  double max_gap = 200;
  double overlap_thresh = 0.7;
  EventNMX evt;

  for (auto nx = x_clusters.clusters.begin(); nx != x_clusters.clusters.end();) {
    // if matched cluster exists and we no longer overlap
    // stash it, increment and reset
    if (!evt.empty() && !evt.time_overlap_thresh(*nx, overlap_thresh)) {
      matched_clusters.emplace_back(std::move(evt));
      stats_cluster_count++;
      evt = EventNMX();
    }

    // it was either emptied or still overlapping
    evt.merge(*nx, 0);
    x_clusters.clusters.erase(nx++);

    for (auto ny = y_clusters.clusters.begin(); ny != y_clusters.clusters.end();) {
      if ((ny->time_start - evt.time_end()) > max_gap)
        break;
      else if (evt.time_overlap_thresh(*ny, overlap_thresh)) {
        evt.merge(*ny, 1);
        y_clusters.clusters.erase(ny++);
      } else
        ny++;
    }
  }

//  x_clusters.clear();
//  y_clusters.clear();
}
