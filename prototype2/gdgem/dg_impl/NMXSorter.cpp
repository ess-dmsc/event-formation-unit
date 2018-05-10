#include <gdgem/dg_impl/NMXSorter.h>
#include <algorithm>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

HitsQueue::HitsQueue(SRSTime Time, double maxTimeGap)
    : pTime(Time), pMaxTimeGap(maxTimeGap) {}

const HitContainer &HitsQueue::hits() const {
  return hitsOut;
}

void HitsQueue::store(uint16_t strip, uint16_t adc, double chipTime) {
  if (chipTime < pTime.max_chip_time_in_window()) {
    hitsNew.emplace_back(Eventlet());
    auto &e = hitsNew[hitsNew.size() - 1];
    e.adc = adc;
    e.strip = strip;
    e.time = chipTime;
  } else {
    hitsOld.emplace_back(Eventlet());
    auto &e = hitsOld[hitsOld.size() - 1];
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
    e.adc = itFind->adc;
    e.strip = itFind->strip;
    e.time = timeNext;
  }

  //Deleting all hits that have been inserted into oldHits (up to itFind, but not including itFind)
  hitsNew.erase(itHitsBegin, itFind);
}

NMXHitSorter::NMXHitSorter(SRSTime time, SRSMappings chips, uint16_t ADCThreshold,
                           double maxTimeGap, NMXClusterer &cb) :
    pTime(time), pChips(chips),
    pADCThreshold(ADCThreshold), callback_(cb),
    hits(pTime, maxTimeGap) {

}

//====================================================================================================================
void NMXHitSorter::store(int triggerTimestamp, unsigned int frameCounter,
                         int fecID, int vmmID, int chNo, int bcid, int tdc, int adc,
                         int overThresholdFlag) {

  // Ready for factoring out, logic tested elsewhere
  uint16_t strip = pChips.get_strip(fecID, vmmID, chNo);

  // Fix for entries with all zeros
  if (bcid == 0 && tdc == 0 && overThresholdFlag) {
    bcid = oldBcid;
    tdc = oldTdc;
    stats_bcid_tdc_error++;
  }
  oldBcid = bcid;
  oldTdc = tdc;
  // oldVmmID = vmmID; // does this need to match for above logic?

  // Could be factored out depending on above block
  double chipTime = pTime.chip_time(bcid, tdc);

  double triggerTimestamp_ns = pTime.timestamp_ns(triggerTimestamp);
  if (oldTriggerTimestamp_ns != triggerTimestamp_ns) {
    stats_trigger_count++;
    analyze();
    hits.subsequent_trigger(false);
    double deltaTriggerTimestamp_ns =
        pTime.delta_timestamp_ns(oldTriggerTimestamp_ns,
                                 triggerTimestamp_ns,
                                 oldFrameCounter,
                                 frameCounter,
                                 stats_triggertime_wraps);

    if (deltaTriggerTimestamp_ns <= pTime.trigger_period()) {
      hits.subsequent_trigger(true); // should this happen before analyze?
    }
  }
  oldTriggerTimestamp_ns = triggerTimestamp_ns;

  // This is likely resolved. Candidate for removal?
  if ((frameCounter < oldFrameCounter)
      && !(oldFrameCounter > frameCounter + 1000000000)) {
    stats_fc_error++;
  }
  oldFrameCounter = frameCounter;

  // Store hit to appropriate buffer
  if (overThresholdFlag || (adc >= pADCThreshold)) {
    hits.store(strip, adc, chipTime);
  }
}

void NMXHitSorter::analyze() {
  hits.sort_and_correct();
  callback_.cluster(hits.hits());
}

