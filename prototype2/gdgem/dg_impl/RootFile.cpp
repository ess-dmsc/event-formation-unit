#include <algorithm>
#include <cmath>
#include <gdgem/dg_impl/RootFile.h>


#define UNUSED __attribute__((unused))

RootFile::RootFile(int bc, int tac, int acqWin,
                   std::vector<int> xChips, std::vector<int> yChips,
                   std::string readout, bool viewEvent, int viewStart,
                   int viewEnd, int threshold, int clusterSize)
    : bcClock(bc), tacSlope(tac), acqWin(acqWin), xChipIDs(xChips),
      yChipIDs(yChips), readoutType(readout), fViewEvent(viewEvent),
      fViewStart(viewStart), fViewEnd(viewEnd), fThreshold(threshold),
      fMinClusterSize(clusterSize) {

  InitRootFile();
}

RootFile::~RootFile() {};

//====================================================================================================================
void RootFile::InitRootFile() {
  m_eventNr = 0;
  m_nch = 0;
  m_nchX = 0;
  m_nchY = 0;

  m_nclX = 0;
  m_nclY = 0;

  m_frameCounter = new unsigned int[max_hit];
  m_vmmID = new unsigned short[max_hit];
  m_overThresholdFlag = new UChar_t[max_hit];
  m_chNo = new unsigned short[max_hit];
  m_x = new unsigned short[max_hit];
  m_y = new unsigned short[max_hit];
  m_adc = new unsigned short[max_hit];
  m_tdc = new unsigned short[max_hit];
  m_bcid = new unsigned short[max_hit];
  m_chipTime = new double[max_hit];
  m_clusterSizeX = new unsigned short[max_hit];
  m_clusterSizeY = new unsigned short[max_hit];
  m_clusterNumberX = new unsigned short[max_hit];
  m_clusterNumberY = new unsigned short[max_hit];
  m_clusterNumberUTPCX = new unsigned short[max_hit];
  m_clusterNumberUTPCY = new unsigned short[max_hit];
  m_clusterX = new float[max_hit];
  m_clusterY = new float[max_hit];
  m_clusterUTPCX = new float[max_hit];
  m_clusterUTPCY = new float[max_hit];
  m_clusterADCX = new unsigned int[max_hit];
  m_clusterTimeX = new float[max_hit];
  m_clusterTimeUTPCX = new float[max_hit];
  m_clusterADCY = new unsigned int[max_hit];
  m_clusterTimeY = new float[max_hit];
  m_clusterTimeUTPCY = new float[max_hit];
  m_clusterDeltaStripX = new float[max_hit];
  m_clusterDeltaStripY = new float[max_hit];
  m_clusterDeltaTimeX = new float[max_hit];
  m_clusterDeltaTimeY = new float[max_hit];
  m_clusterDeltaSpanX = new float[max_hit];
  m_clusterDeltaSpanY = new float[max_hit];
}

//====================================================================================================================
int RootFile::AnalyzeHitData(unsigned int triggerTimestamp,
                             unsigned int frameCounter, unsigned int fecID,
                             unsigned int vmmID, unsigned int chNo,
                             unsigned int bcid, unsigned int tdc,
                             unsigned int adc, unsigned int overThresholdFlag) {

  if (m_eventNr == 0) {
    firstFrameCounter = frameCounter;
  }
  int newEvent = 0;
  triggerTimestamp_ns = triggerTimestamp * 3.125;

  if (oldTriggerTimestamp_ns != triggerTimestamp_ns) {

    newEvent = 1;
    all_hits += m_nch;

    FillClusters();
    subsequentTrigger = 0;
    m_eventNr++;
    if (m_eventNr % MESSAGE_EVENTS == 0) {
      std::cout << "Event: " << m_eventNr << "  -  total hits: " << all_hits
                << std::endl;
    }
  }

  if (oldVmmID != vmmID || newEvent) {
    oldChipTime = 0;
  }

  if ((frameCounter < oldFrameCounter) &&
      !(oldFrameCounter > frameCounter + 1000000000)) {

    std::cout << "*********************************** SCRAMBLED " << m_eventNr
              << " " << oldFrameCounter << " " << frameCounter << std::endl;
    // return -1;
  }

  if (oldTriggerTimestamp_ns > triggerTimestamp_ns &&
      (oldFrameCounter <= frameCounter ||
       oldFrameCounter > frameCounter + 1000000000)) {
    deltaTriggerTimestamp_ns =
        (13421772800 + triggerTimestamp_ns - oldTriggerTimestamp_ns);

  } else {
    deltaTriggerTimestamp_ns = (triggerTimestamp_ns - oldTriggerTimestamp_ns);
  }

  if (newEvent &&
      (deltaTriggerTimestamp_ns <= 1000 * 4096 * (1 / (double)bcClock))) {
    subsequentTrigger = 1;
  }

  if (m_eventNr > 1) {
    timeStamp_ms = timeStamp_ms + deltaTriggerTimestamp_ns * 0.000001;
  }

  planeID = GetPlaneID(vmmID);
  // Plane 0: x
  // plane 1: y
  if (planeID == 0) {
    // Fix for entries with all zeros
    if (bcid == 0 && tdc == 0 && overThresholdFlag) {
      bcid = oldBcidX;
      tdc = oldTdcX;
    }
    oldBcidX = bcid;
    oldTdcX = tdc;
    // lastTimeX[vmmID] = bcTime;

    x = GetChannelX(vmmID, chNo, readoutType);
    y = -1;
  } else if (planeID == 1) {
    // Fix for entries with all zeros
    if (bcid == 0 && tdc == 0 && overThresholdFlag) {
      bcid = oldBcidY;
      tdc = oldTdcY;
    }
    oldBcidY = bcid;
    oldTdcY = tdc;
    y = GetChannelY(vmmID, chNo, readoutType);
    x = -1;
  } else {
    x = -1;
    y = -1;
  }
  // Calculate bcTime [us]
  bcTime = bcid * (1 / (double)bcClock);
  // TDC time: tacSlope * tdc value (8 bit)/ramp length
  // [ns]

  // TDC has reduced resolution due to most significant bit problem of current
  // sources (like ADC)
  int tdcRebinned = (int)tdc / 8;
  tdc = tdcRebinned * 8;
  tdcTime = tacSlope * (double)tdc / 255;

  // Chip time: bcid plus tdc value
  // Talk Vinnie: HIT time  = BCIDx25 + ADC*125/255 [ns]
  chipTime = bcTime * 1000 + tdcTime;

  AddHits(fecID, vmmID, timeStamp_ms, triggerTimestamp_ns, frameCounter,
          overThresholdFlag, chNo, x, y, adc, tdc, bcid, chipTime);

  if (fViewEvent) {
    if (fViewEnd < m_eventNr && fViewEnd != 0) {
      return -1;
    }
    if (printDebugInfo && fViewStart <= m_eventNr && fViewEnd >= m_eventNr) {
      if (newEvent) {
        printf("\neventNr  %d\n", m_eventNr);
        printf("fecID  %d\n", fecID);
      }

      if (deltaTriggerTimestamp_ns > 0) {
        printf("\tTimestamp %.2f [ms]\n", timeStamp_ms);
        printf("\tTime since last trigger %.4f us (%.4f kHz)\n",
               deltaTriggerTimestamp_ns * 0.001,
               1000000 / deltaTriggerTimestamp_ns);
        printf("\tTriggerTimestamp %.2f [ns]\n", triggerTimestamp_ns);
      }
      if (oldFrameCounter != frameCounter || newEvent) {
        printf("\n\tFrameCounter %u\n", frameCounter);
      }
      if (oldVmmID != vmmID || newEvent) {
        printf("\tvmmID  %d\n", vmmID);
      }
      if (planeID == 0) {
        printf("\t\tx-channel %d (chNo  %d) - overThresholdFlag %d\n", x, chNo,
               overThresholdFlag);
      } else if (planeID == 1) {
        printf("\t\ty-channel %d (chNo  %d) - overThresholdFlag %d\n", y, chNo,
               overThresholdFlag);
      } else {
        printf("\t\tPlane for vmmID %d not defined!\n", vmmID);
      }

      printf("\t\t\tbcid %d, tdc %d, adc %d\n", bcid, tdc, adc);
      printf("\t\t\tbcTime %.2f us, tdcTime %.2f ns, time %.2f us\n", bcTime,
             tdcTime, chipTime * 0.001);
    }
  }

  oldTriggerTimestamp_ns = triggerTimestamp_ns;
  oldChipTime = chipTime;
  oldFrameCounter = frameCounter;
  oldVmmID = vmmID;
  return 0;
}

//====================================================================================================================
void RootFile::AddHits(unsigned short UNUSED fecID, unsigned short UNUSED vmmID,
                       double UNUSED timeStamp, double UNUSED triggerTimestamp,
                       unsigned int UNUSED frameCounter, UChar_t overThresholdFlag,
                       unsigned short UNUSED chNo, short x, short y,
                       unsigned short adc, unsigned short UNUSED tdc,
                       unsigned short bcid, double chipTime) {

  if (x > -1 && (adc >= fThreshold || overThresholdFlag)) {

    if (bcid < acqWin * bcClock / 40) {
      hitsX.insert(std::make_pair(chipTime, std::make_pair(x, adc)));

    } else {
      hitsOldX.insert(std::make_pair(chipTime, std::make_pair(x, adc)));
    }
  }
  if (y > -1 && (adc >= fThreshold || overThresholdFlag)) {
    if (bcid < acqWin * bcClock / 40) {
      hitsY.insert(std::make_pair(chipTime, std::make_pair(y, adc)));

    } else {
      hitsOldY.insert(std::make_pair(chipTime, std::make_pair(y, adc)));
    }
  }
}

//====================================================================================================================
int RootFile::ClusterByTime(
    std::multimap<double, std::pair<int, unsigned int>> &oldHits, double dTime,
    double dStrip, double dSpan, string coordinate) {
  m_deltaTime = 0;
  int clusterCount = 0;

  std::multimap<int, std::pair<double, unsigned int>> cluster;

  std::multimap<double, std::pair<int, unsigned int>>::iterator itOldHits =
      oldHits.begin();

  int stripCount = 0;
  double time1 = 0, time2 = 0;
  double deltaT = 0;
  unsigned int adc1 = 0;
  int strip1 = 0;
  cluster.clear();
  for (; itOldHits != oldHits.end(); itOldHits++) {
    time2 = time1;
    time1 = itOldHits->first;
    strip1 = itOldHits->second.first;
    adc1 = itOldHits->second.second;
    deltaT = time1 - time2;
    if (time1 - time2 <= dTime && stripCount > 0) {
      m_deltaTime += deltaT;
    }
    if (time1 - time2 > dTime && stripCount > 0) {
      m_deltaTime = m_deltaTime / stripCount;
      clusterCount += ClusterByStrip(cluster, dStrip, dSpan, coordinate);
      cluster.clear();
      m_deltaTime = 0;
    }
    cluster.insert(std::make_pair(strip1, std::make_pair(time1, adc1)));
    stripCount++;
  }
  if (stripCount > 0) {
    m_deltaTime = m_deltaTime / stripCount;
    clusterCount += ClusterByStrip(cluster, dStrip, dSpan, coordinate);
  }
  return clusterCount;
}

//====================================================================================================================
int RootFile::ClusterByStrip(
    std::multimap<int, std::pair<double, unsigned int>> &cluster, double dStrip,
    double dSpan, string coordinate) {

  m_deltaStrip = 0;
  m_deltaSpan = 0;

  std::multimap<int, std::pair<double, unsigned int>>::iterator itCluster =
      cluster.begin();

  double startTime = 0;
  double largestTime = 0;
  double clusterPositionUTPC = 0;

  double centerOfGravity = 0;
  double centerOfTime = 0;
  unsigned int totalADC = 0;
  double time1 = 0;
  unsigned int adc1 = 0;
  int strip1 = 0, strip2 = 0;
  int stripCount = 0;
  int clusterCount = 0;

  for (; itCluster != cluster.end(); itCluster++) {

    adc1 = itCluster->second.second;
    strip2 = strip1;
    strip1 = itCluster->first;
    time1 = itCluster->second.first;
    // At beginning of cluster, set start time of cluster
    if (stripCount == 0) {
      startTime = time1;
      m_deltaStrip = 0;
      if (printDebugInfo && fViewStart <= m_eventNr && fViewEnd >= m_eventNr) {
        printf("\n%s cluster:\n", coordinate.c_str());
      }
    }
    // Add members of a cluster, if it is either the beginning of a cluster,
    // or if strip gap and time span is correct
    if (stripCount == 0 ||
        (abs(strip1 - strip2) > 0 && abs(strip1 - strip2) <= (dStrip + 1) &&
         time1 - startTime <= dSpan)) {
      if (printDebugInfo && fViewStart <= m_eventNr && fViewEnd >= m_eventNr) {
        printf("\tstrip %d, time %f, adc %d:\n", strip1, time1, adc1);
      }
      if (stripCount > 0) {
        m_deltaStrip += abs(strip1 - strip2);
      }
      if (time1 > largestTime) {
        largestTime = time1;
        clusterPositionUTPC = strip1;
      }
      if (time1 < startTime) {
        startTime = time1;
      }
      m_deltaSpan = (largestTime - startTime);
      centerOfGravity += strip1 * adc1;
      centerOfTime += time1 * adc1;
      totalADC += adc1;
      stripCount++;

    }
    // Stop clustering if gap between strips is too large or time span too long
    else if (abs(strip1 - strip2) > (dStrip + 1) ||
             largestTime - startTime > dSpan) {
      // Valid cluster
      if (stripCount >= fMinClusterSize) {
        m_deltaStrip = m_deltaStrip / (stripCount - 1);
        centerOfGravity = (centerOfGravity / (double)totalADC);
        centerOfTime = (centerOfTime / totalADC);
        AddClusters(centerOfGravity, clusterPositionUTPC, stripCount, totalADC,
                    centerOfTime, largestTime, coordinate);
        clusterCount++;
        if (printDebugInfo && fViewStart <= m_eventNr &&
            fViewEnd >= m_eventNr) {
          printf("******** VALID ********\n");
        }
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
      m_deltaStrip = 0;
      m_deltaSpan = 0;
    }
  }
  // At the end of the clustering, check again if there is a last valid cluster
  if (stripCount >= fMinClusterSize) {
    m_deltaStrip = m_deltaStrip / (stripCount - 1);
    m_deltaSpan = (largestTime - startTime);
    centerOfGravity = (centerOfGravity / (double)totalADC);
    centerOfTime = (centerOfTime / totalADC);
    AddClusters(centerOfGravity, clusterPositionUTPC, stripCount, totalADC,
                centerOfTime, largestTime, coordinate);
    clusterCount++;
    m_deltaStrip = 0;
    m_deltaSpan = 0;
    if (printDebugInfo && fViewStart <= m_eventNr && fViewEnd >= m_eventNr) {
      printf("******** VALID ********\n");
    }
  }
  return clusterCount;
}

//====================================================================================================================
void RootFile::AddClusters(float clusterPosition, float clusterPositionUTPC,
                           short clusterSize, unsigned int clusterADC,
                           float clusterTime, float clusterTimeUTPC,
                           string coordinate) {

  if (m_nclX < max_hit || m_nclY < max_hit) {
    if (coordinate == "x" && clusterPosition > -1.0) {
      m_clusterX[m_nclX] = clusterPosition;
      m_clusterUTPCX[m_nclX] = clusterPositionUTPC;
      m_clusterSizeX[m_nclX] = clusterSize;
      m_clusterADCX[m_nclX] = clusterADC;
      m_clusterTimeX[m_nclX] = clusterTime;
      m_clusterTimeUTPCX[m_nclX] = clusterTimeUTPC;
      m_clusterNumberX[m_nclX] = 0;
      m_clusterNumberUTPCX[m_nclX] = 0;
      m_clusterDeltaTimeX[m_nclX] = m_deltaTime;
      m_clusterDeltaStripX[m_nclX] = m_deltaStrip;
      m_clusterDeltaSpanX[m_nclX] = m_deltaSpan;

      m_nclX++;
    }
    if (coordinate == "y" && clusterPosition > -1.0) {
      m_clusterY[m_nclY] = clusterPosition;
      m_clusterUTPCY[m_nclY] = clusterPositionUTPC;
      m_clusterSizeY[m_nclY] = clusterSize;
      m_clusterADCY[m_nclY] = clusterADC;
      m_clusterTimeY[m_nclY] = clusterTime;
      m_clusterTimeUTPCY[m_nclY] = clusterTimeUTPC;
      m_clusterNumberY[m_nclY] = 0;
      m_clusterNumberUTPCY[m_nclY] = 0;
      m_clusterDeltaTimeY[m_nclY] = m_deltaTime;
      m_clusterDeltaStripY[m_nclY] = m_deltaStrip;
      m_clusterDeltaSpanY[m_nclY] = m_deltaSpan;

      m_nclY++;
    }

  } else {
    std::cout << "ERROR! More than " << max_hit << " clusters produced!"
              << std::endl;
  }
}

//====================================================================================================================
void RootFile::MatchClustersXY(int UNUSED a, int UNUSED b, int UNUSED c, int d) {

  for (unsigned int nx = 0; nx < m_nclX; nx++) {
    double tx = m_clusterTimeX[nx];
    double posx = m_clusterX[nx];

    double oldDeltaT = 999999999999;
    double deltaT = 0;

    for (unsigned int ny = 0; ny < m_nclY; ny++) {
      double ty = m_clusterTimeY[ny];
      double posy = m_clusterY[ny];

      deltaT = abs(ty - tx);

      if (deltaT <= deltaTimePlanes[d] && m_clusterNumberY[ny] == 0 &&
          m_clusterNumberX[nx] == 0) {

        m_clusterNumberY[ny] = 1;
        m_clusterNumberX[nx] = 1;

        m_commonX = posx;
        m_commonY = posy;

        m_commonSizeX = m_clusterSizeX[nx];
        m_commonSizeY = m_clusterSizeY[ny];
        m_commonADCX = m_clusterADCX[nx];
        m_commonADCY = m_clusterADCY[ny];
        m_commonTimeX = tx;
        m_commonTimeY = ty;
        m_deltaTimeX = m_clusterDeltaTimeX[nx];
        m_deltaStripX = m_clusterDeltaStripX[nx];
        m_deltaSpanX = m_clusterDeltaSpanX[nx];
        m_deltaTimeY = m_clusterDeltaTimeY[ny];
        m_deltaStripY = m_clusterDeltaStripY[ny];
        m_deltaSpanY = m_clusterDeltaSpanY[ny];
        m_deltaPlane = tx - ty;

        std::cout << "\ncommon cluster x/y (center of mass):" << std::endl;
        std::cout << "\tpos x/pos y: " << posx << "/" << posy << std::endl;
        std::cout << "\ttime x/time y: " << tx << "/" << ty << std::endl;
        std::cout << "\tadc x/adc y: " << m_clusterADCX[nx] << "/"
                  << m_clusterADCY[ny]
                  << " - ratio:" << 100 * m_clusterADCX[nx] / m_clusterADCY[ny]
                  << std::endl;
        std::cout << "\tsize x/size y: " << m_clusterSizeX[nx] << "/"
                  << m_clusterSizeY[ny] << std::endl;

        break;
      }
      oldDeltaT = deltaT;
    }
  }

  for (unsigned int nx = 0; nx < m_nclX; nx++) {
    double tx = m_clusterTimeUTPCX[nx];
    double posx = m_clusterUTPCX[nx];

    double oldDeltaT = 999999999999;
    double deltaT = 0;

    for (unsigned int ny = 0; ny < m_nclY; ny++) {

      double ty = m_clusterTimeUTPCY[ny];
      double posy = m_clusterUTPCY[ny];

      deltaT = abs(ty - tx);

      if (deltaT <= deltaTimePlanes[d] && m_clusterNumberUTPCY[ny] == 0 &&
          m_clusterNumberUTPCX[nx] == 0) {

        m_clusterNumberUTPCY[ny] = 1;
        m_clusterNumberUTPCX[nx] = 1;

        m_commonX = posx;
        m_commonY = posy;
        m_commonSizeX = m_clusterSizeX[nx];
        m_commonSizeY = m_clusterSizeY[ny];
        m_commonADCX = m_clusterADCX[nx];
        m_commonADCY = m_clusterADCY[ny];
        m_commonTimeX = tx;
        m_commonTimeY = ty;
        m_deltaTimeX = m_clusterDeltaTimeX[nx];
        m_deltaStripX = m_clusterDeltaStripX[nx];
        m_deltaSpanX = m_clusterDeltaSpanX[nx];
        m_deltaTimeY = m_clusterDeltaTimeY[ny];
        m_deltaStripY = m_clusterDeltaStripY[ny];
        m_deltaSpanY = m_clusterDeltaSpanY[ny];
        m_deltaPlane = tx - ty;

        std::cout << "\ncommon cluster x/y (uTPC):" << std::endl;
        std::cout << "\tpos x/pos y: " << posx << "/" << posy << std::endl;
        std::cout << "\ttime x/time y: " << tx << "/" << ty << std::endl;
        std::cout << "\tadc x/adc y: " << m_clusterADCX[nx] << "/"
                  << m_clusterADCY[ny]
                  << " - ratio:" << 100 * m_clusterADCX[nx] / m_clusterADCY[ny]
                  << std::endl;
        std::cout << "\tsize x/size y: " << m_clusterSizeX[nx] << "/"
                  << m_clusterSizeY[ny] << std::endl;

        break;
      }
      oldDeltaT = deltaT;
    }
  }
}

//====================================================================================================================
void RootFile::FillClusters() {
  CorrectTriggerData(hitsX, hitsOldX, deltaTimeHits[MAX_DELTA_TIME_HITS - 1], "x");
  CorrectTriggerData(hitsY, hitsOldY, deltaTimeHits[MAX_DELTA_TIME_HITS - 1], "y");

  for (int a = 0; a < MAX_DELTA_TIME_HITS; a++) {
    for (int b = 0; b < MAX_DELTA_STRIP_HITS; b++) {
      for (int c = 0; c < MAX_DELTA_TIME_SPAN; c++) {
        for (int d = 0; d < MAX_DELTA_TIME_PLANES; d++) {

          std::cout << "\n*****************************************************"
                       "*******************************************************"
                       "****"
                    << std::endl;
          std::cout
              << "Settings:\n"
              << "\tmaximum time difference between neigbouring hits in "
                 "cluster: "
              << deltaTimeHits[a] << " ns\n"
              << "\tmaximum missing strips in cluster: "
              << deltaStripHits[b] - 1 << " strips\n"
              << "\tmaximum time span of hits in cluster (largest time - "
                 "smallest time): "
              << deltaTimeSpan[c] << " ns\n"
              << "\tmaximum time difference between clusters in x/y planes: "
              << deltaTimePlanes[d] << " ns" << std::endl;
          std::cout << "*******************************************************"
                       "*******************************************************"
                       "**"
                    << std::endl;

          int clusterCount =
              ClusterByTime(hitsOldX, deltaTimeHits[a], deltaStripHits[b],
                            deltaTimeSpan[c], "x");
          clusterCount =
              ClusterByTime(hitsOldY, deltaTimeHits[a], deltaStripHits[b],
                            deltaTimeSpan[c], "y");

          MatchClustersXY(a, b, c, d);

          m_nclX = 0;
          m_nclY = 0;
        }
      }
    }
  }

  m_nclX = 0;
  m_nclY = 0;

  hitsOldX.clear();
  hitsOldY.clear();
  if (!hitsX.empty())
    hitsOldX = hitsX;
  if (!hitsY.empty())
    hitsOldY = hitsY;
  hitsX.clear();
  hitsY.clear();
}

//====================================================================================================================
void RootFile::CorrectTriggerData(
    std::multimap<double, std::pair<int, unsigned int>> &hits,
    std::multimap<double, std::pair<int, unsigned int>> &oldHits,
    double correctionTime, string UNUSED coordinate) {
  if (oldHits.size() > 0 && hits.size() > 0) {

    if (subsequentTrigger) {
      std::multimap<double, std::pair<int, unsigned int>>::iterator itBegin =
          hits.begin();
      std::multimap<double, std::pair<int, unsigned int>>::iterator itEnd =
          hits.end();
      std::multimap<double, std::pair<int, unsigned int>>::reverse_iterator
          itReverseEnd = oldHits.rbegin();
      double bcPeriod = 1000 * 4096 * (1 / (double)bcClock);
      double timePrevious = itReverseEnd->first;
      double timeNext = itBegin->first + bcPeriod;
      double deltaTime = timeNext - timePrevious;

      if (deltaTime <= correctionTime) {

        std::multimap<double, std::pair<int, unsigned int>>::iterator it;
        std::multimap<double, std::pair<int, unsigned int>>::iterator itFind;

        for (it = itBegin; it != itEnd; ++it) {
          timePrevious = timeNext;
          timeNext = (*it).first + bcPeriod;
          deltaTime = timeNext - timePrevious;
          itFind = it;
          if (deltaTime > correctionTime) {
            break;
          } else {
            oldHits.insert(std::make_pair(
                timeNext, std::make_pair(it->second.first, it->second.second)));
          }
        }

        hits.erase(hits.begin(), itFind);
      }
    }
  }
}

//====================================================================================================================
unsigned int RootFile::GetPlaneID(unsigned int chipID) {
  std::vector<int>::iterator it;

  it = find(xChipIDs.begin(), xChipIDs.end(), chipID);
  if (it != xChipIDs.end()) {
    return 0;
  } else {
    it = find(yChipIDs.begin(), yChipIDs.end(), chipID);
    if (it != yChipIDs.end()) {
      return 1;
    } else {
      return -1;
    }
  }
}

//====================================================================================================================
unsigned int RootFile::GetChannelX(unsigned int chipID, unsigned int channelID,
                                   std::string UNUSED readout) {
  std::vector<int>::iterator it;

  it = find(xChipIDs.begin(), xChipIDs.end(), chipID);
  if (it != xChipIDs.end()) {
    int pos = it - xChipIDs.begin();
    return channelID + pos * 64;
  } else {
    return -1;
  }
}

//====================================================================================================================
unsigned int RootFile::GetChannelY(unsigned int chipID, unsigned int channelID,
                                   std::string UNUSED readout) {
  std::vector<int>::iterator it;

  it = find(yChipIDs.begin(), yChipIDs.end(), chipID);
  if (it != yChipIDs.end()) {
    int pos = it - yChipIDs.begin();
    return channelID + pos * 64;
  } else {
    return -1;
  }
}
