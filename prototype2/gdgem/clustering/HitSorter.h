/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/clustering/AbstractClusterer.h>
#include <gdgem/srs/SRSTime.h>
#include <gdgem/srs/SRSMappings.h>
#include <gdgem/nmx/Readout.h>
#include <memory>

namespace Gem {

class HitSorter {
public:
  HitSorter(SRSTime time, SRSMappings chips, uint16_t ADCThreshold, double maxTimeGap);

  void insert(const Readout &readout);
  void flush();

  // Statistics counters
  size_t stats_trigger_count{0};

  std::shared_ptr<AbstractClusterer> clusterer;

  void analyze();

  HitContainer buffer;

 private:
  SRSTime pTime;
  SRSMappings pChips;
  uint16_t pADCThreshold;

  // This is in play for triggering the actual clustering
  uint64_t old_trigger_timestamp_{0};
};

}
