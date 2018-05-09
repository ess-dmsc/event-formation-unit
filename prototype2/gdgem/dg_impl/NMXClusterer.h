#pragma once

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include <future>   // async

#include <gdgem/vmm2srs/SRSMappings.h>
#include <gdgem/vmm2srs/SRSTime.h>
#include <gdgem/dg_impl/NMXClusterMatcher.h>

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
  NMXClusterer(SRSTime time, size_t minClusterSize,
               double deltaTimeHits, uint16_t deltaStripHits, double deltaTimeSpan);

  void ClusterByTime(const HitContainer &oldHits);
  void ClusterByStrip(HitContainer &cluster);

  void stash_cluster(PlaneNMX& plane);

  bool ready() const;

  size_t stats_cluster_count{0};

  ClusterVector clusters;

private:
  SRSTime pTime;

  size_t pMinClusterSize;
  double pDeltaTimeHits;
  uint16_t pDeltaStripHits;
  double pDeltaTimeSpan;
};

class NMXHitSorter {
public:
  NMXHitSorter(SRSTime time, SRSMappings chips, uint16_t ADCThreshold, double deltaTimeHits,
               NMXClusterer& cb);

  // Analyzing and storing the hits
  void AnalyzeHits(int triggerTimestamp, unsigned int frameCounter, int fecID,
                   int vmmID, int chNo, int bcid, int tdc, int adc,
                   int overThresholdFlag);

  // Analyzing and storing the clusters
  void AnalyzeClusters();

  // Statistics counters
  size_t stats_fc_error{0};
  size_t stats_bcid_tdc_error{0};
  size_t stats_triggertime_wraps{0};

  HitsQueue hits;

private:
  SRSTime pTime;
  SRSMappings pChips;

  uint16_t pADCThreshold;
  double pDeltaTimeHits;

  // These are in play for triggering the actual clustering
  double m_oldTriggerTimestamp_ns {0};
  unsigned int m_oldFrameCounter {0};

  // For debug output only
  double m_timeStamp_ms {0};
  int m_oldVmmID {0};
  int m_eventNr {0};

  // For all 0s correction
  int m_oldBcid {0};
  int m_oldTdc {0};

  NMXClusterer& callback_;
};
