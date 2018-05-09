#include <algorithm>
#include <cmath>
#include <cassert>
#include <common/Trace.h>
#include <gdgem/dg_impl/NMXClusterMatcher.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

NMXClusterMatcher::NMXClusterMatcher(double dPlane) : pdPlane(dPlane) {
}

void NMXClusterMatcher::match_end(ClusterVector &x_clusters,
                                  ClusterVector &y_clusters) {
  for (ClusterVector::iterator nx = begin(x_clusters);
       nx != end(x_clusters); ++nx) {
    double tx = nx->time_end;

    double minDelta = 99999999;
    double deltaT = 0;
    ClusterVector::iterator it_y = end(y_clusters);

    double ty = 0;

    for (auto ny = y_clusters.begin(); ny != y_clusters.end(); ++ny) {
        ty = ny->time_end;
        deltaT = std::abs(ty - tx);
        if (deltaT < minDelta && deltaT <= pdPlane) {
          minDelta = deltaT;
          it_y = ny;
        }
    }

    if (it_y != end(y_clusters)) {
      DTRACE(DEB, "\ncommon cluster x/y (center of mass):");
      DTRACE(DEB, "\tpos x/pos y: %f/%f", nx->center, it_y->center);
      DTRACE(DEB, "\ttime x/time y: : %f/%f", tx, ty);

      y_clusters.erase(it_y++);
      //x_clusters.erase(nx++);

      //m_clusterXY.emplace_back(std::move(theCommonCluster));
      // callback(theCommonCluster))
      stats_cluster_count++;
    }
  }

  x_clusters.clear();
  y_clusters.clear();
}

void NMXClusterMatcher::match_overlap(ClusterVector &x_clusters,
                                      ClusterVector &y_clusters) {
  for (ClusterVector::iterator nx = begin(x_clusters);
       nx != end(x_clusters); ++nx) {
    double tx = nx->time_end;

    double minDelta = 99999999;
    double deltaT = 0;
    ClusterVector::iterator it_y = end(y_clusters);

    double ty = 0;

    for (auto ny = y_clusters.begin(); ny != y_clusters.end(); ++ny) {
      ty = ny->time_end;
      deltaT = std::abs(ty - tx);
      if (deltaT < minDelta && deltaT <= pdPlane) {
        minDelta = deltaT;
        it_y = ny;
      }
    }

    if (it_y != end(y_clusters)) {
      DTRACE(DEB, "\ncommon cluster x/y (center of mass):");
      DTRACE(DEB, "\tpos x/pos y: %f/%f", nx->center, it_y->center);
      DTRACE(DEB, "\ttime x/time y: : %f/%f", tx, ty);

      y_clusters.erase(it_y++);
      //x_clusters.erase(nx++);

      //m_clusterXY.emplace_back(std::move(theCommonCluster));
      // callback(theCommonCluster))
      stats_cluster_count++;
    }
  }

  x_clusters.clear();
  y_clusters.clear();
}
