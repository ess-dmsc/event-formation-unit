#pragma once

#include <gdgem/clustering/AbstractClusterer.h>
#include <gdgem/clustering/HitsQueue.h>
#include <gdgem/srs/SRSMappings.h>
#include <gdgem/nmx/Readout.h>
#include <memory>

class HitSorter {
public:
  HitSorter(SRSTime time, SRSMappings chips, uint16_t ADCThreshold, double maxTimeGap);

  // TODO: should be by constref
  void insert(Readout readout);
  void flush();

private:
  // These are in play for triggering the actual clustering
  double old_trigger_timestamp_ns_ {0};
  uint32_t old_frame_counter_ {0};

  // For all 0s correction
  uint16_t old_bcid_ {0};
  uint16_t old_tdc_ {0};

  SRSTime pTime;
  SRSMappings pChips;
  uint16_t pADCThreshold;

  void analyze();

public:
  // Statistics counters
  size_t stats_fc_error{0};
  size_t stats_bcid_tdc_error{0};
  size_t stats_triggertime_wraps{0};
  size_t stats_trigger_count {0};

  std::shared_ptr<AbstractClusterer> clusterer;

  // TODO private?
  HitsQueue hits;
};
