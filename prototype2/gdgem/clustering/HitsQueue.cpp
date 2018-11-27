/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <gdgem/clustering/HitsQueue.h>
#include <algorithm>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

namespace Gem {

HitsQueue::HitsQueue(SRSTime Time)
    : pTime(Time) {}

const HitContainer &HitsQueue::hits() const {
  return hitsOut;
}

/// \todo sort out Hit constr
void HitsQueue::store(uint8_t plane, uint16_t strip, uint16_t adc,
                      float chipTime, uint64_t trigger_time) {
  hitsOut.push_back(Hit());
  auto &e = hitsOut.back();
  e.plane = plane;
  e.weight = adc;
  e.coordinate = strip;
  e.time = trigger_time  + static_cast<uint64_t>(chipTime);
}

void HitsQueue::sort_and_correct() {
  /// \todo What are the sizes of (number of elements) in hitsOld and hitsNew?
  /// \todo This might be relevant to the type of sorting that should be used.
  std::sort(hitsOut.begin(), hitsOut.end(),
            [](const Hit &e1, const Hit &e2) {
              return e1.time < e2.time;
            });
}

void HitsQueue::clear() {
  hitsOut.clear();
}

}
