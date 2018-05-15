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
    stats_trigger_count++;

    double delta_trigger_ns =
        pTime.delta_timestamp_ns(old_trigger_timestamp_ns_,
                                 triggerTimestamp_ns,
                                 old_frame_counter_,
                                 readout.frame_counter,
                                 stats_triggertime_wraps);

    hits.subsequent_trigger(delta_trigger_ns <= pTime.trigger_period_ns());

    analyze();
  }
  old_trigger_timestamp_ns_ = triggerTimestamp_ns;

  // TODO: This is likely resolved. Candidate for removal?
  if ((readout.frame_counter < old_frame_counter_)
      && !(old_frame_counter_ > readout.frame_counter + 1000000000)) {
    stats_fc_error++;
  }
  old_frame_counter_ = readout.frame_counter;


  // Register as error, but don't do correction
  if (readout.bcid == 0 && readout.tdc == 0 && readout.over_threshold) {
    stats_bcid_tdc_error++;
  }

  // TODO: add test for if adc=0 && over_threshold, and report

  if (readout.over_threshold || (readout.adc >= pADCThreshold)) {
    hits.store(pChips.get_plane(readout), pChips.get_strip(readout),
               readout.adc, pTime.chip_time_ns(readout.bcid, readout.tdc));
    // TODO: who adds chipTime + trigger time? queue?
  }
}

void HitSorter::flush() {
  //flush both buffers in queue
  // TODO: subsequent trigger? How do we know?
//  hits.subsequent_trigger(true);
  analyze();
//  hits.subsequent_trigger(true);
  analyze();
}

void HitSorter::analyze() {
  hits.sort_and_correct();
  if (clusterer)
    clusterer->cluster(hits.hits());
}

