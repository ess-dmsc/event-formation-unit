#include <gdgem/clustering/HitSorter.h>
#include <algorithm>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

HitSorter::HitSorter(SRSTime time, SRSMappings chips, uint16_t ADCThreshold,
                           double maxTimeGap, std::shared_ptr<AbstractClusterer> cb) :
    pTime(time), pChips(chips),
    pADCThreshold(ADCThreshold), callback_(cb),
    hits(pTime, maxTimeGap) {

}

//====================================================================================================================
void HitSorter::insert(Readout readout) {

  // TODO: factor this out?
  // Fix for entries with all zeros
  if (readout.bcid == 0 && readout.tdc == 0 && readout.over_threshold) {
    readout.bcid = old_bcid_;
    readout.tdc = old_tdc_;
    stats_bcid_tdc_error++;
  }
  old_bcid_ = readout.bcid;
  old_tdc_ = readout.tdc;
  // oldVmmID = vmmID; // does this need to match for above logic?

  // Could be factored out depending on above block
  double chipTime = pTime.chip_time(readout.bcid, readout.tdc);

  double triggerTimestamp_ns = pTime.timestamp_ns(readout.srs_timestamp);
  if (old_trigger_timestamp_ns_ != triggerTimestamp_ns) {
    stats_trigger_count++;
    analyze();
    hits.subsequent_trigger(false);
    double deltaTriggerTimestamp_ns =
        pTime.delta_timestamp_ns(old_trigger_timestamp_ns_,
                                 triggerTimestamp_ns,
                                 old_frame_counter_,
                                 readout.frame_counter,
                                 stats_triggertime_wraps);

    if (deltaTriggerTimestamp_ns <= pTime.trigger_period()) {
      hits.subsequent_trigger(true); // should this happen before analyze?
    }
  }
  old_trigger_timestamp_ns_ = triggerTimestamp_ns;

  // This is likely resolved. Candidate for removal?
  if ((readout.frame_counter < old_frame_counter_)
      && !(old_frame_counter_ > readout.frame_counter + 1000000000)) {
    stats_fc_error++;
  }
  old_frame_counter_ = readout.frame_counter;

  // Store hit to appropriate buffer
  if (readout.over_threshold || (readout.adc >= pADCThreshold)) {
    hits.store(pChips.get_plane(readout.fec, readout.chip_id),
               pChips.get_strip(readout.fec, readout.chip_id, readout.channel),
               readout.adc, chipTime);
  }
}

void HitSorter::flush() {
  //flush both buffers in queue
  analyze();
  analyze();
}

void HitSorter::analyze() {
  hits.sort_and_correct();
  if (callback_)
    callback_->cluster(hits.hits());
}

