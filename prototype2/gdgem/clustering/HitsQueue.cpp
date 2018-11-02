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

HitsQueue::HitsQueue(SRSTime Time, double maxTimeGap)
    : pTime(Time), pMaxTimeGap(maxTimeGap) {}

const HitContainer &HitsQueue::hits() const {
  return hitsOut.buffer;
}

/// \todo sort out Hit constr
void HitsQueue::store(uint8_t plane, uint16_t strip, uint16_t adc,
                      double chipTime, double trigger_time) {
  HitBuffer *b = &hitsOld;
  if (chipTime < pTime.max_chip_time_in_window_ns()) {
    b = &hitsNew;
  }
  b->trigger_time = trigger_time;
  b->buffer.push_back(Hit());
  auto &e = b->buffer.back();
  e.plane_id = plane;
  e.adc = adc;
  e.strip = strip;
  e.time = chipTime;
}

void HitsQueue::sort_and_correct() {
  /// \todo What are the sizes of (number of elements) in hitsOld and hitsNew?
  /// \todo This might be relevant to the type of sorting that should be used.
  std::sort(hitsOld.buffer.begin(), hitsOld.buffer.end(),
            [](const Hit &e1, const Hit &e2) {
              return e1.time < e2.time;
            });

  std::sort(hitsNew.buffer.begin(), hitsNew.buffer.end(),
            [](const Hit &e1, const Hit &e2) {
              return e1.time < e2.time;
            });
  correct_trigger_data();

  hitsOut = std::move(hitsOld);
  hitsOld = std::move(hitsNew);

  if (!hitsNew.buffer.empty()) {
    hitsNew.buffer.clear();
  }

  for (auto &h : hitsOut.buffer)
    h.time += hitsOut.trigger_time;
}

void HitsQueue::subsequent_trigger(bool trig) {
  subsequent_trigger_ = trig;
}

void HitsQueue::correct_trigger_data() {
  if (!subsequent_trigger_)
    return;
  // TODO Does this happen? Does it really mean do nothing?
  if (hitsNew.buffer.empty() || hitsOld.buffer.empty())
    return;
  double latestOld = hitsOld.buffer.rbegin()->time; // Latest of the old
  // oldest of the new + correct into time space of the old
  double timeNext = hitsNew.buffer.begin()->time + pTime.trigger_period_ns();
  double deltaTime = timeNext - latestOld;
  //Continue only if the first hit in hits is close enough in time to the last hit in oldHits
  if (deltaTime > pMaxTimeGap)
    return;

  double timePrevious;
  HitContainer::iterator itFind;
  //Loop through all hits in hits
  for (itFind = hitsNew.buffer.begin(); itFind != hitsNew.buffer.end(); ++itFind) {
    //At the first iteration, timePrevious is set to the time of the first hit in hits
    timePrevious = timeNext;
    //At the first iteration, timeNext is again set to the time of the first hit in hits
    // + correct into time space of the old
    timeNext = itFind->time + pTime.trigger_period_ns();

    //At the first iteration, delta time is 0
    deltaTime = timeNext - timePrevious;

    if (deltaTime > pMaxTimeGap)
      break;

    hitsOld.buffer.emplace_back(Hit());
    auto &e = hitsOld.buffer[hitsOld.buffer.size() - 1];
    e.plane_id = itFind->plane_id;
    e.adc = itFind->adc;
    e.strip = itFind->strip;
    e.time = timeNext;
  }

  //Deleting all hits that have been inserted into oldHits (up to itFind, but not including itFind)
  hitsNew.buffer.erase(hitsNew.buffer.begin(), itFind);

// TODO Confirm that this is not needed
  std::sort(hitsOld.buffer.begin(), hitsOld.buffer.end(),
            [](const Hit &e1, const Hit &e2) {
              return e1.time < e2.time;
            });

  // TODO if al hits were transferred, flag edge as possibly invalid
}

}
