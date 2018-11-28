/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <gdgem/clustering/HitSorter.h>
#include <algorithm>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

namespace Gem {

HitSorter::HitSorter(SRSTime time, SRSMappings chips, uint16_t ADCThreshold,
                     double maxTimeGap) :
    pTime(time), pChips(chips),
    pADCThreshold(ADCThreshold) {
  (void) maxTimeGap;

}

void HitSorter::insert(const Readout &readout) {

  if (old_trigger_timestamp_ != readout.srs_timestamp) {
    stats_trigger_count++;
  }
  old_trigger_timestamp_ = readout.srs_timestamp;

  /// \todo Move this check to parser or builder?
  if (readout.over_threshold || (readout.adc >= pADCThreshold)) {

    buffer.push_back(Hit());
    auto &e = buffer.back();
    e.plane = pChips.get_plane(readout);
    e.weight = readout.adc;
    e.coordinate = pChips.get_strip(readout);
    e.time = readout.srs_timestamp + static_cast<uint64_t>(readout.chiptime);
  }
}

void HitSorter::flush() {
  analyze();
  if (clusterer)
    clusterer->flush();
}

void HitSorter::analyze() {

  std::sort(buffer.begin(), buffer.end(),
            [](const Hit &e1, const Hit &e2) {
              return e1.time < e2.time;
            });

  if (clusterer)
    clusterer->cluster(buffer);
  buffer.clear();
}

}
