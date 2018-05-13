#include <gdgem/clustering/HitsQueue.h>
#include <algorithm>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

HitsQueue::HitsQueue(SRSTime Time, double maxTimeGap)
    : pTime(Time), pMaxTimeGap(maxTimeGap) {}

const HitContainer &HitsQueue::hits() const {
  return hitsOut;
}

void HitsQueue::store(uint8_t plane, uint16_t strip, uint16_t adc, double chipTime) {
  if (chipTime < pTime.max_chip_time_in_window()) {
    hitsNew.emplace_back(Eventlet());
    auto &e = hitsNew[hitsNew.size() - 1];
    e.plane_id = plane;
    e.adc = adc;
    e.strip = strip;
    e.time = chipTime;
  } else {
    hitsOld.emplace_back(Eventlet());
    auto &e = hitsOld[hitsOld.size() - 1];
    e.plane_id = plane;
    e.adc = adc;
    e.strip = strip;
    e.time = chipTime;
  }
}

void HitsQueue::sort_and_correct() {
  std::sort(hitsOld.begin(), hitsOld.end(),
            [](const Eventlet &e1, const Eventlet &e2) {
              return e1.time < e2.time;
            });

  std::sort(hitsNew.begin(), hitsNew.end(),
            [](const Eventlet &e1, const Eventlet &e2) {
              return e1.time < e2.time;
            });
  correct_trigger_data();

  hitsOut = std::move(hitsOld);
  hitsOld = std::move(hitsNew);

  std::sort(hitsOut.begin(), hitsOut.end(),
            [](const Eventlet &e1, const Eventlet &e2) {
              return e1.time < e2.time;
            });


  if (!hitsNew.empty()) {
    hitsNew.clear();
  }
}

void HitsQueue::subsequent_trigger(bool trig) {
  subsequent_trigger_ = trig;
}

void HitsQueue::correct_trigger_data() {
  if (!subsequent_trigger_)
    return;

  const auto &itHitsBegin = begin(hitsNew);
  const auto &itHitsEnd = end(hitsNew);
  const auto &itOldHitsBegin = hitsOld.rend();
  const auto &itOldHitsEnd = hitsOld.rbegin();

  // If either list is empty
  if (itHitsBegin == itHitsEnd || itOldHitsBegin == itOldHitsEnd)
    return;

  double timePrevious = itOldHitsEnd->time; // Newest of the old
  // oldest of the new + correct into time space of the old
  double timeNext = itHitsBegin->time + pTime.trigger_period();
  double deltaTime = timeNext - timePrevious;
  //Continue only if the first hit in hits is close enough in time to the last hit in oldHits
  if (deltaTime > pMaxTimeGap)
    return;

  HitContainer::iterator itFind;
  //Loop through all hits in hits
  for (itFind = itHitsBegin; itFind != itHitsEnd; ++itFind) {
    //At the first iteration, timePrevious is sett to the time of the first hit in hits
    timePrevious = timeNext;
    //At the first iteration, timeNext is again set to the time of the first hit in hits
    // + correct into time space of the old
    timeNext = itFind->time + pTime.trigger_period();

    //At the first iteration, delta time is 0
    deltaTime = timeNext - timePrevious;

    if (deltaTime > pMaxTimeGap)
      break;

    hitsOld.emplace_back(Eventlet());
    auto &e = hitsNew[hitsNew.size() - 1];
    e.plane_id = itFind->plane_id;
    e.adc = itFind->adc;
    e.strip = itFind->strip;
    e.time = timeNext;
  }

  //Deleting all hits that have been inserted into oldHits (up to itFind, but not including itFind)
  hitsNew.erase(itHitsBegin, itFind);
}
