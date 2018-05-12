#pragma once

#include <gdgem/clustering/AbstractClusterer.h>
#include <gdgem/clustering/HitsQueue.h>
#include <gdgem/srs/SRSMappings.h>
#include <memory>

class HitSorter {
public:
  HitSorter(SRSTime time, SRSMappings chips, uint16_t ADCThreshold, double maxTimeGap,
            std::shared_ptr<AbstractClusterer> cb);

  // Analyzing and storing the hits
  void store(int triggerTimestamp, unsigned int frameCounter, int fecID,
             int vmmID, int chNo, int bcid, int tdc, int adc,
             int overThresholdFlag);

  void flush();

private:
  // These are in play for triggering the actual clustering
  double oldTriggerTimestamp_ns {0};
  unsigned int oldFrameCounter {0};

  // For all 0s correction
  int oldBcid {0};
  int oldTdc {0};

  SRSTime pTime;
  SRSMappings pChips;
  uint16_t pADCThreshold;
  std::shared_ptr<AbstractClusterer> callback_;

  void analyze();

public:
  // Statistics counters
  size_t stats_fc_error{0};
  size_t stats_bcid_tdc_error{0};
  size_t stats_triggertime_wraps{0};
  size_t stats_trigger_count {0};

  HitsQueue hits;

};
