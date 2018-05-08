#pragma once

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include <future>   // async

#include <gdgem/vmm2srs/SRSMappings.h>
#include <gdgem/vmm2srs/SRSTime.h>
#include <gdgem/nmx/Eventlet.h>

using std::string;

struct Hit
{
	unsigned int fec;
	unsigned int chip_id;
	unsigned int framecounter;
	unsigned int srs_timestamp;
	unsigned int channel;
	unsigned int bcid;
	unsigned int tdc;
	unsigned int adc;
	unsigned int overthreshold;
};

struct ClusterNMX {
  int size;
  int adc;
  double position;
  double time;
  bool clusterXAndY;
  double maxDeltaTime;
  double maxDeltaStrip;
  double deltaSpan;
};

struct CommonClusterNMX {
  int sizeX;
  int sizeY;
  int adcX;
  int adcY;
  double timeX;
  double timeY;
  double deltaPlane;
};

//using HitTuple = std::tuple<double, int, int>;
using HitTuple = Eventlet;
using HitContainer = std::vector<HitTuple>;
//using ClusterTuple = std::tuple<int, double, int>;
using ClusterTuple = Eventlet;
using ClusterContainer = std::vector<ClusterTuple>;
using ClusterVector = std::vector<ClusterNMX>;


class HitsQueue {
public:
  HitsQueue(SRSTime Time, double deltaTimeHits);
  void store(uint16_t strip, uint16_t adc, double chipTime);
  void sort_and_correct();
  void CorrectTriggerData();
  void subsequentTrigger(bool);

  const HitContainer& hits() const;

private:
  HitContainer hitsOld;
  HitContainer hitsNew;
  HitContainer hitsOut;

  SRSTime pTime;
  double pDeltaTimeHits {200};
  bool m_subsequentTrigger {false};
};


class NMXClusterer {
public:
  NMXClusterer(SRSTime time,
               SRSMappings chips,
               uint16_t adcThreshold, size_t minClusterSize,
               double deltaTimeHits, uint16_t deltaStripHits, double deltaTimeSpan,
               double deltaTimePlanes);

  ~NMXClusterer();

  // Analyzing and storing the hits
  bool AnalyzeHits(int triggerTimestamp, unsigned int frameCounter, int fecID,
                   int vmmID, int chNo, int bcid, int tdc, int adc,
                   int overThresholdFlag);

  // Analyzing and storing the clusters
  void AnalyzeClusters();

  size_t ClusterByTime(const HitContainer &oldHits, double dTime, int dStrip,
                    double dSpan, string coordinate);
  size_t ClusterByStrip(ClusterContainer &cluster, int dStrip, double dSpan,
                     string coordinate, double maxDeltaTime);

  void StoreClusters(double clusterPosition,
                     short clusterSize, int clusterADC, double clusterTime,
                     string coordinate, double maxDeltaTime, int maxDeltaStrip, double deltaSpan);

  void MatchClustersXY(double dPlane);

  int getNumClustersX() {
    return m_clusterX_size;
  };
  int getNumClustersY() {
    return m_clusterY_size;
  };
  int getNumClustersXY() {
    return m_clusterXY_size;
  };

  // Statistics counters
  uint64_t stats_fc_error{0};
  uint64_t stats_bcid_tdc_error{0};
  uint64_t stats_triggertime_wraps{0};

  const uint8_t planeID_X{0};
  const uint8_t planeID_Y{1};

private:
  SRSTime pTime;
  SRSMappings pChips;

  uint16_t pADCThreshold;
  size_t pMinClusterSize;
  double pDeltaTimeHits;
  uint16_t pDeltaStripHits;
  double pDeltaTimeSpan;
  double pDeltaTimePlanes;

  // These are in play for triggering the actual clustering
  double m_oldTriggerTimestamp_ns = 0;
  unsigned int m_oldFrameCounter = 0;

  // For debug output only
  double m_timeStamp_ms = 0;
  int m_oldVmmID = 0;
  int m_eventNr = 0;

  // For all 0s correction
  int m_oldBcidX = 0;
  int m_oldTdcX = 0;
  int m_oldBcidY = 0;
  int m_oldTdcY = 0;

  HitsQueue hitsX;
  HitsQueue hitsY;

  // std::vector<CommonClusterNMX> m_clusterXY;
  std::vector<ClusterNMX> m_tempClusterX;
  std::vector<ClusterNMX> m_tempClusterY;
  //std::vector<ClusterNMX> m_clusterX;
  //std::vector<ClusterNMX> m_clusterY;

  uint64_t m_clusterXY_size{0};
  uint64_t m_clusterX_size{0};
  uint64_t m_clusterY_size{0};
};
