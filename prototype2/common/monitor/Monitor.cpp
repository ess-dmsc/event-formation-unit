/** Copyright (C) 2016-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <common/monitor/Monitor.h>

#include <common/Log.h>
//#undef TRC_MASK
//#define TRC_MASK 0

Monitor::Monitor(const std::string& broker,
    const std::string& topic_prefix,
    const std::string& source_name) {
  source_name_ = source_name;
  producer = std::make_shared<Producer>(broker, topic_prefix + "_monitor");
}

void Monitor::init_histograms(size_t max_range) {
  histograms = std::make_shared<Hists>(max_range, max_range);
  hist_serializer = std::make_shared<HistogramSerializer>(
      histograms->needed_buffer_size(), source_name_);

#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
  hist_serializer->set_callback(std::bind(&Producer::produce2<uint8_t>, producer.get(), std::placeholders::_1));
#pragma GCC diagnostic pop
}

void Monitor::init_hits(size_t max_readouts) {
  hit_serializer = std::make_shared<HitSerializer>(
      max_readouts, source_name_);

#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
  hit_serializer->set_callback(std::bind(&Producer::produce2<uint8_t>, producer.get(), std::placeholders::_1));
#pragma GCC diagnostic pop
}

void Monitor::close() {
  histograms.reset();
  hit_serializer.reset();
  hist_serializer.reset();
  producer.reset();
}

void Monitor::produce_now() {
  if (!producer)
    return;

  if (!histograms->isEmpty()) {
    LOG(PROCESS, Sev::Debug, "Flushing histograms for {} readouts", histograms->hit_count());
    hist_serializer->produce(*histograms);
    histograms->clear();
  }

  if (hit_serializer->getNumEntries()) {
    LOG(PROCESS, Sev::Debug, "Flushing readout data for {} readouts", hit_serializer->getNumEntries());
    hit_serializer->produce();
  }
}
