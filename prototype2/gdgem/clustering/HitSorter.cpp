#include <gdgem/clustering/HitSorter.h>
#include <algorithm>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

HitSorter::HitSorter(SRSTime time, SRSMappings chips, uint16_t ADCThreshold,
                           double maxTimeGap) :
    pTime(time), pChips(chips),
    pADCThreshold(ADCThreshold),
    hits(pTime, maxTimeGap) {

}

//====================================================================================================================
void HitSorter::insert(Readout readout) {

  double triggerTimestamp_ns = pTime.trigger_timestamp_ns(readout.srs_timestamp);
  if (old_trigger_timestamp_ns_ != triggerTimestamp_ns) {

    analyze();

    double delta_trigger_ns =
        pTime.delta_timestamp_ns(old_trigger_timestamp_ns_,
                                 triggerTimestamp_ns,
                                 old_frame_counter_,
                                 readout.frame_counter,
                                 stats_triggertime_wraps);

    // TODO: should this happen before analyze?
    hits.subsequent_trigger(delta_trigger_ns <= pTime.trigger_period());

    stats_trigger_count++;
  }
  old_trigger_timestamp_ns_ = triggerTimestamp_ns;

  // TODO: This is likely resolved. Candidate for removal?
  if ((readout.frame_counter < old_frame_counter_)
      && !(old_frame_counter_ > readout.frame_counter + 1000000000)) {
    stats_fc_error++;
  }
  old_frame_counter_ = readout.frame_counter;


  // TODO: factor this out?
  // Fix for entries with all zeros
  if (readout.bcid == 0 && readout.tdc == 0 && readout.over_threshold) {
    readout.bcid = old_bcid_;
    readout.tdc = old_tdc_;
    stats_bcid_tdc_error++;
  }
  old_bcid_ = readout.bcid;
  old_tdc_ = readout.tdc;
  // TODO: should this include oldVmmID = vmmID, do they need to match?
  // Could be factored out depending on above block
  double chipTime = pTime.chip_time(readout.bcid, readout.tdc);

  if (readout.over_threshold || (readout.adc >= pADCThreshold)) {
    // TODO: if adc=0 && over_threshold, adc=dumm_val?
    hits.store(pChips.get_plane(readout), pChips.get_strip(readout),
               readout.adc, chipTime);
    // TODO: who adds chipTime + trigger time? queue?
  }
}

void HitSorter::flush() {
  //flush both buffers in queue
  // TODO: subsequent trigger? How do we know?
  analyze();
  analyze();
}

void HitSorter::analyze() {
  hits.sort_and_correct();
  if (clusterer)
    clusterer->cluster(hits.hits());
}

