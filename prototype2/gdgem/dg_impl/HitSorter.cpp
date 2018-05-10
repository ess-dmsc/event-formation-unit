#include <gdgem/dg_impl/HitSorter.h>
#include <algorithm>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

HitSorter::HitSorter(SRSTime time, SRSMappings chips, uint16_t ADCThreshold,
                           double maxTimeGap, NMXClusterer &cb) :
    pTime(time), pChips(chips),
    pADCThreshold(ADCThreshold), callback_(cb),
    hits(pTime, maxTimeGap) {

}

//====================================================================================================================
void HitSorter::store(int triggerTimestamp, unsigned int frameCounter,
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

void HitSorter::analyze() {
  hits.sort_and_correct();
  callback_.cluster(hits.hits());
}

