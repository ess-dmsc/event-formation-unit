#include <algorithm>
#include <cmath>
#include <cassert>
#include <common/Trace.h>
#include <gdgem/dg_impl/NMXClusterMatcher.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

NMXClusterMatcher::NMXClusterMatcher(double dPlane) : pdPlane(dPlane)
{
}

void NMXClusterMatcher::match(std::vector<ClusterNMX>& x_clusters, std::vector<ClusterNMX>& y_clusters)
{
  for (auto &nx : x_clusters) {
    nx.plane.analyze(true, 10000, 10000);

    double tx = nx.plane.time_start;
    double posx = nx.plane.center;

    double minDelta = 99999999;
    double deltaT = 0;
    ClusterVector::iterator it = end(y_clusters);

    double ty = 0;
    double posy = 0;

    for (ClusterVector::iterator ny = begin(y_clusters);
         ny != end(y_clusters); ++ny) {
      if ((*ny).clusterXAndY == false) {
        ty = (*ny).plane.time_start;
        deltaT = std::abs(ty - tx);
        if (deltaT < minDelta && deltaT <= pdPlane) {
          minDelta = deltaT;
          it = ny;
        }
      }
    }
    if (it != end(y_clusters)) {
      nx.clusterXAndY = true;
      (*it).clusterXAndY = true;

      CommonClusterNMX theCommonCluster;
      theCommonCluster.sizeX = nx.plane.entries.size();
      theCommonCluster.sizeY = (*it).plane.entries.size();
      theCommonCluster.adcX = nx.plane.integral;
      theCommonCluster.adcY = (*it).plane.integral;
      theCommonCluster.timeX = nx.plane.time_start;
      theCommonCluster.timeY = (*it).plane.time_start;
      theCommonCluster.deltaPlane = std::abs(theCommonCluster.timeX - theCommonCluster.timeY);

      DTRACE(DEB, "\ncommon cluster x/y (center of mass):");
      DTRACE(DEB, "\tpos x/pos y: %f/%f", posx, posy);
      DTRACE(DEB, "\ttime x/time y: : %f/%f", tx, ty);
      DTRACE(DEB, "\tadc x/adc y: %u/%u", theCommonCluster.adcX,
             theCommonCluster.adcY);
      DTRACE(DEB, "\tsize x/size y: %u/%u", theCommonCluster.sizeX,
             theCommonCluster.sizeY);
      DTRACE(DEB, "\tdelta time planes: %f", theCommonCluster.deltaPlane);
      //m_clusterXY.emplace_back(std::move(theCommonCluster));
      // callback(theCommonCluster))
      stats_cluster_count++;
    }
  }

  x_clusters.clear();
  y_clusters.clear();
}
