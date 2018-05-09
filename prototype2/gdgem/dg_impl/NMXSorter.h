#pragma once

#include <gdgem/dg_impl/NMXClusterer.h>
#include <gdgem/vmm2srs/SRSMappings.h>

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

private:
  SRSTime pTime;
  SRSMappings pChips;

  uint16_t pADCThreshold;

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

public:
  // Statistics counters
  size_t stats_fc_error{0};
  size_t stats_bcid_tdc_error{0};
  size_t stats_triggertime_wraps{0};

  HitsQueue hits;
};
