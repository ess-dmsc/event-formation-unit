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
  float position;
  float time;
  bool clusterXAndY;
  float maxDeltaTime;
  float maxDeltaStrip;
  float deltaSpan;
};

struct CommonClusterNMX {
  int sizeX;
  int sizeY;
  int adcX;
  int adcY;
  float timeX;
  float timeY;
  float deltaPlane;
};

using HitTuple = std::tuple<float, int, int>;
using HitContainer = std::vector<HitTuple>;
using ClusterTuple = std::tuple<int, float, int>;
using ClusterContainer = std::vector<ClusterTuple>;
using ClusterVector = std::vector<ClusterNMX>;


class HitsQueue {
public:
  HitsQueue(SRSTime Time, float deltaTimeHits);
  void store(uint16_t strip, short adc, float chipTime);
  void sort_and_correct();
  void CorrectTriggerData(HitContainer &hits, HitContainer &oldHits, float correctionTime);
  void subsequentTrigger(bool);

  const HitContainer& hits() const;

private:
  HitContainer hitsOld;
  HitContainer hitsNew;
  HitContainer hitsOut;

  SRSTime pTime;
  float pDeltaTimeHits {200};
  bool m_subsequentTrigger {false};
};


class NMXClusterer {
public:
  NMXClusterer(SRSTime time,
               SRSMappings chips,
               int adcThreshold, int minClusterSize,
               float deltaTimeHits, int deltaStripHits, float deltaTimeSpan,
               float deltaTimePlanes);

  ~NMXClusterer();

  // Analyzing and storing the hits
  bool AnalyzeHits(int triggerTimestamp, unsigned int frameCounter, int fecID,
                   int vmmID, int chNo, int bcid, int tdc, int adc,
                   int overThresholdFlag);

  // Analyzing and storing the clusters
  void AnalyzeClusters();

  int ClusterByTime(const HitContainer &oldHits, float dTime, int dStrip,
                    float dSpan, string coordinate);
  int ClusterByStrip(ClusterContainer &cluster, int dStrip, float dSpan,
                     string coordinate, float maxDeltaTime);

  void StoreClusters(float clusterPosition,
                     short clusterSize, int clusterADC, float clusterTime,
                     string coordinate, float maxDeltaTime, int maxDeltaStrip, float deltaSpan);

  void MatchClustersXY(float dPlane);

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

  int pADCThreshold;
  int pMinClusterSize;
  float pDeltaTimeHits;
  int pDeltaStripHits;
  float pDeltaTimeSpan;
  float pDeltaTimePlanes;

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
