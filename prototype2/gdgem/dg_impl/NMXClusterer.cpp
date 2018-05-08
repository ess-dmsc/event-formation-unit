#include <algorithm>
#include <cmath>
#include <cassert>
#include <common/Trace.h>
#include <gdgem/dg_impl/NMXClusterer.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

HitsQueue::HitsQueue(SRSTime Time, double deltaTimeHits)
    : pTime(Time), pDeltaTimeHits(deltaTimeHits) {}

const HitContainer& HitsQueue::hits() const
{
  return hitsOut;
}

void HitsQueue::store(uint16_t strip, uint16_t adc, double chipTime) {
  if (chipTime < pTime.max_chip_time_in_window()) {
    hitsNew.emplace_back(Eventlet());
    auto &e = hitsNew[hitsNew.size() - 1];
    e.adc = adc;
    e.strip = strip;
    e.time = chipTime;
  } else {
    hitsOld.emplace_back(Eventlet());
    auto &e = hitsOld[hitsOld.size() - 1];
    e.adc = adc;
    e.strip = strip;
    e.time = chipTime;
  }
}

void HitsQueue::sort_and_correct()
{
  std::sort(begin(hitsOld), end(hitsOld),
            [](const Eventlet &e1, const Eventlet &e2) {
              return e1.time <= e2.time;
            });

  std::sort(begin(hitsNew), end(hitsNew),
            [](const Eventlet &e1, const Eventlet &e2) {
              return e1.time <= e2.time;
            });
  CorrectTriggerData();

  hitsOut = std::move(hitsOld);

  hitsOld = std::move(hitsNew);
  if (!hitsNew.empty()) {
    hitsNew.clear();
  }
}

void HitsQueue::subsequentTrigger(bool trig)
{
  m_subsequentTrigger = trig;
}

void HitsQueue::CorrectTriggerData() {
  if (!m_subsequentTrigger)
    return;

  const auto &itHitsBegin = begin(hitsNew);
  const auto &itHitsEnd = end(hitsNew);
  const auto &itOldHitsBegin = hitsOld.rend();
  const auto &itOldHitsEnd = hitsOld.rbegin();

  // If either list is empty
  if (itHitsBegin == itHitsEnd || itOldHitsBegin == itOldHitsEnd)
    return;

  double timePrevious = itOldHitsEnd->time; // Newest of the old
  // oldest of the new + correct into time space of the old
  double timeNext = itHitsBegin->time + pTime.trigger_period();
  double deltaTime = timeNext - timePrevious;
  //Continue only if the first hit in hits is close enough in time to the last hit in oldHits
  if (deltaTime > pDeltaTimeHits)
    return;

  HitContainer::iterator itFind;
  //Loop through all hits in hits
  for (itFind = itHitsBegin; itFind != itHitsEnd; ++itFind) {
    //At the first iteration, timePrevious is sett to the time of the first hit in hits
    timePrevious = timeNext;
    //At the first iteration, timeNext is again set to the time of the first hit in hits
    // + correct into time space of the old
    timeNext = itFind->time + pTime.trigger_period();

    //At the first iteration, delta time is 0
    deltaTime = timeNext - timePrevious;

    if (deltaTime > pDeltaTimeHits)
      break;

    hitsOld.emplace_back(Eventlet());
    auto &e = hitsNew[hitsNew.size() - 1];
    e.adc = itFind->adc;
    e.strip = itFind->strip;
    e.time = timeNext;
  }

  //Deleting all hits that have been inserted into oldHits (up to itFind, but not including itFind)
  hitsNew.erase(itHitsBegin, itFind);
}




NMXClusterer::NMXClusterer(SRSTime time, SRSMappings chips,
                           int adcThreshold, int minClusterSize, double deltaTimeHits,
                           int deltaStripHits, double deltaTimeSpan, double deltaTimePlanes
                           /* callback() */ ) :
    pTime(time), pChips(chips), pADCThreshold(
    adcThreshold), pMinClusterSize(minClusterSize), pDeltaTimeHits(
    deltaTimeHits), pDeltaStripHits(deltaStripHits), pDeltaTimeSpan(
    deltaTimeSpan), pDeltaTimePlanes(deltaTimePlanes), m_eventNr(0),
    hitsX(pTime, deltaTimeHits), hitsY(pTime, deltaTimeHits)
{
}

NMXClusterer::~NMXClusterer() {
}

//====================================================================================================================
bool NMXClusterer::AnalyzeHits(int triggerTimestamp, unsigned int frameCounter,
                               int fecID, int vmmID, int chNo, int bcid, int tdc, int adc,
                               int overThresholdFlag) {

  // Ready for factoring out, logic tested elsewhere
  uint8_t planeID = pChips.get_plane(fecID, vmmID);
  uint16_t strip = pChips.get_strip(fecID, vmmID, chNo);

  // These variables are used only here
  // Block is candidate for factoring out
  // Perhaps an adapter class responsible for recovery from this error condition?
  if (planeID == planeID_X) {
    // Fix for entries with all zeros
    if (bcid == 0 && tdc == 0 && overThresholdFlag) {
      bcid = m_oldBcidX;
      tdc = m_oldTdcX;
      stats_bcid_tdc_error++;
    }
    m_oldBcidX = bcid;
    m_oldTdcX = tdc;
  } else if (planeID == planeID_Y) {
    // Fix for entries with all zeros
    if (bcid == 0 && tdc == 0 && overThresholdFlag) {
      bcid = m_oldBcidY;
      tdc = m_oldTdcY;
      stats_bcid_tdc_error++;
    }
    m_oldBcidY = bcid;
    m_oldTdcY = tdc;
  }

  // Could be factored out depending on above block
  double chipTime = pTime.chip_time(bcid, tdc);

  bool newEvent = false;
  double deltaTriggerTimestamp_ns = 0;
  double triggerTimestamp_ns = pTime.timestamp_ns(triggerTimestamp);

  if (m_oldTriggerTimestamp_ns != triggerTimestamp_ns) {
    AnalyzeClusters();
    newEvent = true;
    hitsX.subsequentTrigger(false);
    hitsY.subsequentTrigger(false);
    m_eventNr++;
    deltaTriggerTimestamp_ns = pTime.delta_timestamp_ns(
        m_oldTriggerTimestamp_ns, triggerTimestamp_ns,
        m_oldFrameCounter, frameCounter, stats_triggertime_wraps);

    if (deltaTriggerTimestamp_ns <= pTime.trigger_period()) {
      hitsX.subsequentTrigger(true);
      hitsY.subsequentTrigger(true);
    }
  }

  // Crucial step
  // Storing hit to appropriate buffer
  if (overThresholdFlag || (adc >= pADCThreshold)) {
    if (planeID == planeID_X) {
      hitsX.store(strip, adc, chipTime);
    } else if (planeID == planeID_Y) {
      hitsY.store(strip, adc, chipTime);
    }
  }

  if (newEvent) {
    DTRACE(DEB, "\neventNr  %d\n", m_eventNr);
    DTRACE(DEB, "fecID  %d\n", fecID);
  }

  // This is likely resolved. Candidate for removal?
  if ((frameCounter < m_oldFrameCounter)
      && !(m_oldFrameCounter > frameCounter + 1000000000)) {
    DTRACE(DEB,
           "\n*********************************** SCRAMBLED eventNr  %d, "
           "old framecounter %d, new framecounter %u\n", m_eventNr,
           m_oldFrameCounter, frameCounter);
    stats_fc_error++;
  }

  // m_timeStamp_ms is used for printing Trace info
  if (m_eventNr > 1) {
    m_timeStamp_ms = m_timeStamp_ms + deltaTriggerTimestamp_ns * 0.000001;
  }
  if (deltaTriggerTimestamp_ns > 0) {
    DTRACE(DEB, "\tTimestamp %.2f [ms]\n", m_timeStamp_ms);
    DTRACE(DEB, "\tTime since last trigger %.4f us (%.4f kHz)\n",
           deltaTriggerTimestamp_ns * 0.001,
           1000000 / deltaTriggerTimestamp_ns);
    DTRACE(DEB, "\tTriggerTimestamp %.2f [ns]\n", triggerTimestamp_ns);
  }
  if (m_oldFrameCounter != frameCounter || newEvent) {
    DTRACE(DEB, "\n\tFrameCounter %u\n", frameCounter);
  }
  if (m_oldVmmID != vmmID || newEvent) {
    DTRACE(DEB, "\tvmmID  %d\n", vmmID);
  }
  if (planeID == planeID_X) {
    DTRACE(DEB, "\t\tx-channel %d (chNo  %d) - overThresholdFlag %d\n",
           strip, chNo, overThresholdFlag);
  } else if (planeID == planeID_Y) {
    DTRACE(DEB, "\t\ty-channel %d (chNo  %d) - overThresholdFlag %d\n",
           strip, chNo, overThresholdFlag);
  } else {
    DTRACE(DEB, "\t\tPlane for vmmID %d not defined!\n", vmmID);
  }
  DTRACE(DEB, "\t\t\tbcid %d, tdc %d, adc %d\n", bcid, tdc, adc);
  DTRACE(DEB, "\t\t\tchipTime %.2f us\n", chipTime * 0.001);

  m_oldTriggerTimestamp_ns = triggerTimestamp_ns;
  m_oldFrameCounter = frameCounter;
  m_oldVmmID = vmmID;
  return true;
}

//====================================================================================================================
int NMXClusterer::ClusterByTime(const HitContainer &oldHits, double dTime, int dStrip,
                                double dSpan, string coordinate) {

  ClusterContainer cluster;
  double maxDeltaTime = 0;
  int clusterCount = 0;
  int stripCount = 0;
  double time1 = 0, time2 = 0;
  int adc1 = 0;
  int strip1 = 0;

  for (const auto &itOldHits : oldHits) {
    time2 = time1;

    time1 = itOldHits.time;
    strip1 = itOldHits.strip;
    adc1 = itOldHits.adc;

    if (time1 - time2 <= dTime && stripCount > 0
        && maxDeltaTime < (time1 - time2)) {
      maxDeltaTime = (time1 - time2);
    }

    if (time1 - time2 > dTime && stripCount > 0) {
      clusterCount += ClusterByStrip(cluster, dStrip, dSpan, coordinate,
                                     maxDeltaTime);
      cluster.clear();
      maxDeltaTime = 0;
    }
    cluster.emplace_back(strip1, time1, adc1);
    stripCount++;
  }

  if (stripCount > 0) {
    clusterCount += ClusterByStrip(cluster, dStrip, dSpan, coordinate,
                                   maxDeltaTime);
  }
  return clusterCount;
}

//====================================================================================================================
int NMXClusterer::ClusterByStrip(ClusterContainer &cluster, int dStrip,
                                 double dSpan, string coordinate, double maxDeltaTime) {
  int maxDeltaStrip = 0;
  double deltaSpan = 0;

  double startTime = 0;
  double largestTime = 0;

  double centerOfGravity = -1;
  double centerOfTime = 0;
  int totalADC = 0;
  double time1 = 0;
  int adc1 = 0;
  int strip1 = 0, strip2 = 0;
  int stripCount = 0;
  int clusterCount = 0;

  std::sort(begin(cluster), end(cluster),
            [](const ClusterTuple &t1, const ClusterTuple &t2) {
              return std::get<0>(t1) < std::get<0>(t2);
            });

  for (auto &itCluster : cluster) {
    strip2 = strip1;
    strip1 = std::get<0>(itCluster);
    time1 = std::get<1>(itCluster);
    adc1 = std::get<2>(itCluster);

    // At beginning of cluster, set start time of cluster
    if (stripCount == 0) {
      maxDeltaStrip = 0;
      startTime = time1;
      DTRACE(DEB, "\n%s cluster:\n", coordinate.c_str());
    }

    // Add members of a cluster, if it is either the beginning of a cluster,
    // or if strip gap and time span is correct
    if (stripCount == 0
        || (std::abs(strip1 - strip2) > 0
            && std::abs(strip1 - strip2) <= (dStrip + 1)
            && time1 - startTime <= dSpan)) {
      DTRACE(DEB, "\tstrip %d, time %f, adc %d:\n", strip1, time1, adc1);
      largestTime = std::max(time1, largestTime);
      startTime = std::min(time1, startTime);
      if (stripCount > 0 && maxDeltaStrip < std::abs(strip1 - strip2)) {
        maxDeltaStrip = std::abs(strip1 - strip2);
      }
      deltaSpan = (largestTime - startTime);
      centerOfGravity += strip1 * adc1;
      centerOfTime += time1 * adc1;
      totalADC += adc1;
      stripCount++;
    }
      // Stop clustering if gap between strips is too large or time span too long
    else if (std::abs(strip1 - strip2) > (dStrip + 1)
        || largestTime - startTime > dSpan) {
      // Valid cluster
      if (stripCount >= pMinClusterSize) {
        centerOfGravity = (centerOfGravity / (double) totalADC);
        centerOfTime = (centerOfTime / (double) totalADC);
        StoreClusters(centerOfGravity, stripCount,
                      totalADC, centerOfTime, coordinate,
                      maxDeltaTime, maxDeltaStrip, deltaSpan);
        clusterCount++;
        DTRACE(DEB, "******** VALID ********\n");
        maxDeltaStrip = 0;

      }

      // Reset all parameters
      startTime = 0;
      largestTime = 0;
      stripCount = 0;
      centerOfGravity = 0;
      centerOfTime = 0;
      totalADC = 0;
      strip1 = 0;
    }
  }
  // At the end of the clustering, check again if there is a last valid cluster
  if (stripCount >= pMinClusterSize) {
    deltaSpan = (largestTime - startTime);
    centerOfGravity = (centerOfGravity / (double) totalADC);
    centerOfTime = (centerOfTime / totalADC);
    StoreClusters(centerOfGravity, stripCount,
                  totalADC, centerOfTime, coordinate, maxDeltaTime,
                  maxDeltaStrip, deltaSpan);
    clusterCount++;
    DTRACE(DEB, "******** VALID ********\n");
  }
  return clusterCount;
}
//====================================================================================================================
void NMXClusterer::StoreClusters(double clusterPosition, short clusterSize, int clusterADC,
                                 double clusterTime, string coordinate,
                                 double maxDeltaTime, int maxDeltaStrip, double deltaSpan) {

  ClusterNMX theCluster;
  theCluster.size = clusterSize;
  theCluster.adc = clusterADC;
  theCluster.time = clusterTime;
  theCluster.position = clusterPosition;
  theCluster.clusterXAndY = false;
  theCluster.maxDeltaTime = maxDeltaTime;
  theCluster.maxDeltaStrip = maxDeltaStrip;
  theCluster.deltaSpan = deltaSpan; // rename to something with time

  if (coordinate == "x" && clusterPosition > -1.0) {
    m_tempClusterX.emplace_back(std::move(theCluster));
  }
  if (coordinate == "y" && clusterPosition > -1.0) {
    m_tempClusterY.emplace_back(std::move(theCluster));
  }

}

//====================================================================================================================
void NMXClusterer::MatchClustersXY(double dPlane) {

  for (auto &nx : m_tempClusterX) {
    double tx = nx.time;
    double posx = nx.position;

    double minDelta = 99999999;
    double deltaT = 0;
    ClusterVector::iterator it = end(m_tempClusterY);

    double ty = 0;
    double posy = 0;

    for (ClusterVector::iterator ny = begin(m_tempClusterY);
         ny != end(m_tempClusterY); ++ny) {
      if ((*ny).clusterXAndY == false) {
        ty = (*ny).time;
        deltaT = std::abs(ty - tx);
        if (deltaT < minDelta && deltaT <= dPlane) {
          minDelta = deltaT;
          it = ny;
        }
      }
    }
    if (it != end(m_tempClusterY)) {
      nx.clusterXAndY = true;
      (*it).clusterXAndY = true;

      CommonClusterNMX theCommonCluster;
      theCommonCluster.sizeX = nx.size;
      theCommonCluster.sizeY = (*it).size;
      theCommonCluster.adcX = nx.adc;
      theCommonCluster.adcY = (*it).adc;
      theCommonCluster.timeX = nx.time;
      theCommonCluster.timeY = (*it).time;
      theCommonCluster.deltaPlane = theCommonCluster.timeX
          - theCommonCluster.timeY;

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
      m_clusterXY_size++;
    }
  }

}

//====================================================================================================================
void NMXClusterer::AnalyzeClusters() {
  hitsX.sort_and_correct();

  int cntX = ClusterByTime(hitsX.hits(), pDeltaTimeHits, pDeltaStripHits,
                           pDeltaTimeSpan, "x");

  DTRACE(DEB, "%d cluster in x\n", cntX);

  hitsY.sort_and_correct();

  int cntY = ClusterByTime(hitsY.hits(), pDeltaTimeHits, pDeltaStripHits,
                           pDeltaTimeSpan, "y");

  DTRACE(DEB, "%d cluster in y\n", cntY);

  MatchClustersXY(pDeltaTimePlanes);

  // m_clusterX.insert(m_clusterX.end(),
  //                   std::make_move_iterator(m_tempClusterX.begin()),
  //                   std::make_move_iterator(m_tempClusterX.end()));
  //
  // m_clusterY.insert(m_clusterY.end(),
  //                   std::make_move_iterator(m_tempClusterY.begin()),
  //                   std::make_move_iterator(m_tempClusterY.end()));

  m_clusterX_size += m_tempClusterX.size();
  m_clusterY_size += m_tempClusterY.size();
  m_tempClusterX.clear();
  m_tempClusterY.clear();
}
