#include <algorithm>
#include <cmath>
#include <cassert>
#include <common/Trace.h>
#include <gdgem/dg_impl/NMXClusterer.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

NMXClusterer::NMXClusterer(SRSTime time, SRSMappings chips, int acqWin,
                           int adcThreshold, int minClusterSize, float deltaTimeHits,
                           int deltaStripHits, float deltaTimeSpan, float deltaTimePlanes) :
    pTime(time), pChips(chips), pAcqWin(acqWin), pADCThreshold(
    adcThreshold), pMinClusterSize(minClusterSize), pDeltaTimeHits(
    deltaTimeHits), pDeltaStripHits(deltaStripHits), pDeltaTimeSpan(
    deltaTimeSpan), pDeltaTimePlanes(deltaTimePlanes), m_eventNr(0) {
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
    m_subsequentTrigger = false;
    m_eventNr++;
    deltaTriggerTimestamp_ns = pTime.delta_timestamp_ns(
        m_oldTriggerTimestamp_ns, triggerTimestamp_ns,
        m_oldFrameCounter, frameCounter, stats_triggertime_wraps);

    if (deltaTriggerTimestamp_ns <= pTime.trigger_period()) {
      m_subsequentTrigger = true;
    }

  }

  // Crucial step
  // Storing hit to appropriate buffer
  if (overThresholdFlag || (adc >= pADCThreshold)) {
    if (planeID == planeID_X) {
      StoreX(strip, adc, bcid, chipTime);
    } else if (planeID == planeID_Y) {
      StoreY(strip, adc, bcid, chipTime);
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
void NMXClusterer::StoreX(uint16_t strip, short adc, short bcid,
                          float chipTime) {
  if (bcid < pAcqWin * pTime.bc_clock() / 40) {
    m_hitsX.emplace_back(chipTime, strip, adc);
  } else {
    m_hitsOldX.emplace_back(chipTime, strip, adc);
  }
}

//====================================================================================================================
void NMXClusterer::StoreY(uint16_t strip, short adc, short bcid,
                          float chipTime) {
  if (bcid < pAcqWin * pTime.bc_clock() / 40) {
    m_hitsY.emplace_back(chipTime, strip, adc);
  } else {
    m_hitsOldY.emplace_back(chipTime, strip, adc);
  }
}

//====================================================================================================================
int NMXClusterer::ClusterByTime(HitContainer &oldHits, float dTime, int dStrip,
                                float dSpan, string coordinate) {

  ClusterContainer cluster;
  float maxDeltaTime = 0;
  int clusterCount = 0;
  int stripCount = 0;
  double time1 = 0, time2 = 0;
  int adc1 = 0;
  int strip1 = 0;

  for (auto &itOldHits : oldHits) {
    time2 = time1;

    time1 = std::get<0>(itOldHits);
    strip1 = std::get<1>(itOldHits);
    adc1 = std::get<2>(itOldHits);

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
                                 float dSpan, string coordinate, float maxDeltaTime) {
  int maxDeltaStrip = 0;
  float deltaSpan = 0;

  float startTime = 0;
  float largestTime = 0;
  float clusterPositionUTPC = -1;

  float centerOfGravity = -1;
  float centerOfTime = 0;
  int totalADC = 0;
  float time1 = 0;
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
      if (time1 > largestTime) {
        largestTime = time1;
        clusterPositionUTPC = strip1;
      }
      if (time1 < startTime) {
        startTime = time1;
      }
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
        centerOfGravity = (centerOfGravity / (float) totalADC);
        centerOfTime = (centerOfTime / (float) totalADC);
        StoreClusters(centerOfGravity, clusterPositionUTPC, stripCount,
                      totalADC, centerOfTime, largestTime, coordinate,
                      maxDeltaTime, maxDeltaStrip, deltaSpan);
        clusterCount++;
        DTRACE(DEB, "******** VALID ********\n");
        maxDeltaStrip = 0;

      }

      // Reset all parameters
      startTime = 0;
      largestTime = 0;
      clusterPositionUTPC = 0;
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
    centerOfGravity = (centerOfGravity / (float) totalADC);
    centerOfTime = (centerOfTime / totalADC);
    StoreClusters(centerOfGravity, clusterPositionUTPC, stripCount,
                  totalADC, centerOfTime, largestTime, coordinate, maxDeltaTime,
                  maxDeltaStrip, deltaSpan);
    clusterCount++;
    DTRACE(DEB, "******** VALID ********\n");
  }
  return clusterCount;
}
//====================================================================================================================
void NMXClusterer::StoreClusters(float clusterPosition,
                                 float clusterPositionUTPC, short clusterSize, int clusterADC,
                                 float clusterTime, float clusterTimeUTPC, string coordinate,
                                 float maxDeltaTime, int maxDeltaStrip, float deltaSpan) {

  ClusterNMX theCluster;
  theCluster.size = clusterSize;
  theCluster.adc = clusterADC;
  theCluster.time = clusterTime;
  theCluster.time_uTPC = clusterTimeUTPC;
  theCluster.position = clusterPosition;
  theCluster.position_uTPC = clusterPositionUTPC;
  theCluster.clusterXAndY = false;
  theCluster.clusterXAndY_uTPC = false;
  theCluster.maxDeltaTime = maxDeltaTime;
  theCluster.maxDeltaStrip = maxDeltaStrip;
  theCluster.deltaSpan = deltaSpan;

  if (coordinate == "x" && clusterPosition > -1.0) {
    m_tempClusterX.emplace_back(std::move(theCluster));
  }
  if (coordinate == "y" && clusterPosition > -1.0) {
    m_tempClusterY.emplace_back(std::move(theCluster));
  }

}

//====================================================================================================================
void NMXClusterer::MatchClustersXY(float dPlane) {

  for (auto &nx : m_tempClusterX) {
    float tx = nx.time;
    float posx = nx.position;
    float tx_uTPC = nx.time_uTPC;
    float posx_uTPC = nx.position_uTPC;

    float minDelta = 99999999;
    float deltaT = 0;
    ClusterVector::iterator it = end(m_tempClusterY);

    float ty = 0;
    float posy = 0;

    float minDelta_uTPC = 99999999;
    float deltaT_uTPC = 0;
    ClusterVector::iterator it_uTPC = end(m_tempClusterY);
    float ty_uTPC = 0;
    float posy_uTPC = 0;

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
      if ((*ny).clusterXAndY_uTPC == false) {
        ty_uTPC = (*ny).time_uTPC;
        deltaT_uTPC = std::abs(ty_uTPC - tx_uTPC);
        if (deltaT_uTPC < minDelta_uTPC && deltaT_uTPC <= dPlane) {
          minDelta_uTPC = deltaT_uTPC;
          it_uTPC = ny;
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
      theCommonCluster.positionX = nx.position;
      theCommonCluster.positionY = (*it).position;
      theCommonCluster.timeX = nx.time;
      theCommonCluster.timeY = (*it).time;
      theCommonCluster.deltaPlane = theCommonCluster.timeX
          - theCommonCluster.timeY;

      theCommonCluster.maxDeltaTimeX = nx.maxDeltaTime;
      theCommonCluster.maxDeltaTimeY = (*it).maxDeltaTime;
      theCommonCluster.maxDeltaStripX = nx.maxDeltaStrip;
      theCommonCluster.maxDeltaStripY = (*it).maxDeltaStrip;

      DTRACE(DEB, "\ncommon cluster x/y (center of mass):");
      DTRACE(DEB, "\tpos x/pos y: %f/%f", posx, posy);
      DTRACE(DEB, "\ttime x/time y: : %f/%f", tx, ty);
      DTRACE(DEB, "\tadc x/adc y: %u/%u", theCommonCluster.adcX,
             theCommonCluster.adcY);
      DTRACE(DEB, "\tsize x/size y: %u/%u", theCommonCluster.sizeX,
             theCommonCluster.sizeY);
      DTRACE(DEB, "\tdelta time planes: %f", theCommonCluster.deltaPlane);
      m_clusterXY.emplace_back(std::move(theCommonCluster));

    }
    if (it_uTPC != end(m_tempClusterY)) {
      nx.clusterXAndY_uTPC = true;
      (*it_uTPC).clusterXAndY_uTPC = true;

      CommonClusterNMX theCommonCluster_uTPC;
      theCommonCluster_uTPC.sizeX = nx.size;
      theCommonCluster_uTPC.sizeY = (*it_uTPC).size;
      theCommonCluster_uTPC.adcX = nx.adc;
      theCommonCluster_uTPC.adcY = (*it_uTPC).adc;
      theCommonCluster_uTPC.positionX = nx.position_uTPC;
      theCommonCluster_uTPC.positionY = (*it_uTPC).position_uTPC;
      theCommonCluster_uTPC.timeX = nx.time_uTPC;
      theCommonCluster_uTPC.timeY = (*it_uTPC).time_uTPC;
      theCommonCluster_uTPC.deltaPlane = theCommonCluster_uTPC.timeX
          - theCommonCluster_uTPC.timeY;
      theCommonCluster_uTPC.maxDeltaTimeX = nx.maxDeltaTime;
      theCommonCluster_uTPC.maxDeltaTimeY = (*it_uTPC).maxDeltaTime;
      theCommonCluster_uTPC.maxDeltaStripX = nx.maxDeltaStrip;
      theCommonCluster_uTPC.maxDeltaStripY = (*it_uTPC).maxDeltaStrip;

      DTRACE(DEB, "\ncommon cluster x/y (uTPC):");
      DTRACE(DEB, "\tpos x/pos y: %f/%f", posx_uTPC, posy_uTPC);
      DTRACE(DEB, "\ttime x/time y: : %f/%f", tx_uTPC, ty_uTPC);
      DTRACE(DEB, "\tadc x/adc y: %u/%u", theCommonCluster_uTPC.adcX,
             theCommonCluster_uTPC.adcY);
      DTRACE(DEB, "\tsize x/size y: %u/%u", theCommonCluster_uTPC.sizeX,
             theCommonCluster_uTPC.sizeY);
      DTRACE(DEB, "\tdelta time planes: %f",
             theCommonCluster_uTPC.deltaPlane);
      m_clusterXY_uTPC.emplace_back(std::move(theCommonCluster_uTPC));

    }
  }

}

//====================================================================================================================
void NMXClusterer::AnalyzeClusters() {
  //auto fX = std::async(std::launch::async, [&] {
  std::sort(begin(m_hitsOldX), end(m_hitsOldX),
            [](const ClusterTuple &t1, const ClusterTuple &t2) {
              return std::get<0>(t1) < std::get<0>(t2);
            });

  std::sort(begin(m_hitsX), end(m_hitsX),
            [](const ClusterTuple &t1, const ClusterTuple &t2) {
              return std::get<0>(t1) < std::get<0>(t2);
            });

  CorrectTriggerData(m_hitsX, m_hitsOldX, pDeltaTimeHits);
  int cntX = ClusterByTime(m_hitsOldX, pDeltaTimeHits, pDeltaStripHits,
                           pDeltaTimeSpan, "x");

  DTRACE(DEB, "%d cluster in x\n", cntX);
  m_hitsOldX = std::move(m_hitsX);
  if (!m_hitsX.empty()) {
    m_hitsX.clear();
  }
  //});

  //auto fY = std::async(std::launch::async, [&] {

  std::sort(begin(m_hitsOldY), end(m_hitsOldY),
            [](const ClusterTuple &t1, const ClusterTuple &t2) {
              return std::get<0>(t1) < std::get<0>(t2);
            });

  std::sort(begin(m_hitsY), end(m_hitsY),
            [](const ClusterTuple &t1, const ClusterTuple &t2) {
              return std::get<0>(t1) < std::get<0>(t2);
            });

  CorrectTriggerData(m_hitsY, m_hitsOldY, pDeltaTimeHits);
  int cntY = ClusterByTime(m_hitsOldY, pDeltaTimeHits, pDeltaStripHits,
                           pDeltaTimeSpan, "y");

  DTRACE(DEB, "%d cluster in y\n", cntY);
  m_hitsOldY = std::move(m_hitsY);
  if (!m_hitsY.empty()) {
    m_hitsY.clear();
  }
  //});

  //fX.get();
  //fY.get();

  MatchClustersXY(pDeltaTimePlanes);

  m_clusterX.insert(m_clusterX.end(),
                    std::make_move_iterator(m_tempClusterX.begin()),
                    std::make_move_iterator(m_tempClusterX.end()));
  m_clusterY.insert(m_clusterY.end(),
                    std::make_move_iterator(m_tempClusterY.begin()),
                    std::make_move_iterator(m_tempClusterY.end()));

  m_tempClusterX.clear();
  m_tempClusterY.clear();
}

//====================================================================================================================
void NMXClusterer::CorrectTriggerData(HitContainer &hits, HitContainer &oldHits,
                                      float correctionTime) {
  if (m_subsequentTrigger) {

    const auto &itHitsBegin = begin(hits);
    const auto &itHitsEnd = end(hits);
    const auto &itOldHitsBegin = oldHits.rend();
    const auto &itOldHitsEnd = oldHits.rbegin();
    if (itHitsBegin != itHitsEnd && itOldHitsBegin != itOldHitsEnd) {
      float bcPeriod = 1000 * 4096 / static_cast<float>(pTime.bc_clock());
      float timePrevious = std::get<0>(*itOldHitsEnd);
      float timeNext = std::get<0>(*itHitsBegin) + bcPeriod;
      float deltaTime = timeNext - timePrevious;
      //Code only executed if the first hit in hits is close enough in time to the last hit in oldHits
      if (deltaTime <= correctionTime) {
        HitContainer::iterator itFind;
        //Loop through all hits in hits
        for (itFind = itHitsBegin; itFind != itHitsEnd; ++itFind) {
          //At the first iteration, timePrevious is sett to the time of the first hit in hits
          timePrevious = timeNext;
          //At the first iteration, timeNext is again set to the time of the first hit in hits
          timeNext = std::get<0>(*itFind) + bcPeriod;

          //At the first iteration, delta time is 0
          deltaTime = timeNext - timePrevious;

          if (deltaTime > correctionTime) {
            break;
          } else {
            oldHits.emplace_back(timeNext, std::get<1>(*itFind),
                                 std::get<2>(*itFind));
          }
        }
//Deleting all hits that have been inserted into oldHits (up to itFind, but not including itFind)
        hits.erase(itHitsBegin, itFind);

      }
    }
  }
}

