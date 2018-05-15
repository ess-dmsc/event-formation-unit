#pragma once

#include <gdgem/clustering/AbstractClusterer.h>
#include <gdgem/clustering/HitsQueue.h>
#include <gdgem/srs/SRSMappings.h>
#include <gdgem/nmx/Readout.h>
#include <memory>

class HitSorter {
public:
  HitSorter(SRSTime time, SRSMappings chips, uint16_t ADCThreshold, double maxTimeGap);

  // TODO: would be better by constref?
  void insert(Readout readout);
  void flush();

  // Statistics counters
  size_t stats_trigger_count {0};
  size_t stats_subsequent_triggers {0};

  std::shared_ptr<AbstractClusterer> clusterer;

private:
  SRSTime pTime;
  SRSMappings pChips;
  uint16_t pADCThreshold;
  HitsQueue hits;

  HitContainer buffer;

  // These are in play for triggering the actual clustering
  double old_trigger_timestamp_ns_ {0};

  void analyze();
};
