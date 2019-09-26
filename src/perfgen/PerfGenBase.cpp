/** Copyright (C) 2017-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Implementation of the detector pipeline plugin for MUlti-Blade
/// detectors.
//===----------------------------------------------------------------------===//

#include "PerfGenBase.h"

#include <cinttypes>
#include <common/EFUArgs.h>
#include <common/EV42Serializer.h>
#include <common/Producer.h>
#include <common/monitor/HistogramSerializer.h>
#include <common/Trace.h>
#include <common/TimeString.h>
#include <common/TestImageUdder.h>

#include <unistd.h>

#include <common/SPSCFifo.h>
#include <common/Socket.h>
#include <common/TSCTimer.h>
#include <common/Timer.h>

#include <logical_geometry/ESSGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace PerfGen {

const char *classname = "Multiblade detector with CAEN readout";

PerfGenBase::PerfGenBase(BaseSettings const &settings, struct PerfGenSettings &LocalPerfGenSettings)
    : Detector("PerfGen", settings), PerfGenSettings(LocalPerfGenSettings) {

  Stats.setPrefix(EFUSettings.GraphitePrefix, EFUSettings.GraphiteRegion);

  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off

  Stats.create("thread.processing_idle", mystats.rx_idle1);

  Stats.create("events.count", mystats.events);
  Stats.create("events.udder", mystats.events_udder);
  Stats.create("transmit.bytes", mystats.tx_bytes);

  /// \todo below stats are common to all detectors and could/should be moved
  Stats.create("kafka.produce_fails", mystats.kafka_produce_fails);
  Stats.create("kafka.ev_errors", mystats.kafka_ev_errors);
  Stats.create("kafka.ev_others", mystats.kafka_ev_others);
  Stats.create("kafka.dr_errors", mystats.kafka_dr_errors);
  Stats.create("kafka.dr_others", mystats.kafka_dr_noerrors);
  // clang-format on

  std::function<void()> processingFunc = [this]() {
    PerfGenBase::processing_thread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");
}

void PerfGenBase::processing_thread() {

  if (EFUSettings.KafkaTopic == "") {
    EFUSettings.KafkaTopic = "PERFGEN_detector";
  }

  EV42Serializer flatbuffer(kafka_buffer_size, "multiblade");
  Producer eventprod(EFUSettings.KafkaBroker, EFUSettings.KafkaTopic);
#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
  flatbuffer.setProducerCallback(
      std::bind(&Producer::produce2<uint8_t>, &eventprod, std::placeholders::_1));
#pragma GCC diagnostic pop

  if ( true ) {
    XTRACE(PROCESS, ALW, "GENERATING TEST IMAGE!");
    Udder udder;
    uint32_t time_of_flight = 0;
    while (true) {
      if (not runThreads) {
        // \todo flush everything here
        XTRACE(INPUT, ALW, "Stopping processing thread.");
        return;
      }

      static int eventCount = 0;
      if (eventCount == 0) {
        uint64_t efu_time = 1000000000LU * (uint64_t)time(NULL); // ns since 1970
        flatbuffer.pulseTime(efu_time);
      }

      ESSGeometry essgeom(64, 64, 1, 1);
      auto pixel_id = udder.getPixel(essgeom.nx(), essgeom.ny(), &essgeom);
      auto tx_bytes = flatbuffer.addEvent(time_of_flight, pixel_id);
      mystats.tx_bytes += tx_bytes;
      mystats.events_udder++;

      if (EFUSettings.TestImageUSleep != 0) {
        usleep(EFUSettings.TestImageUSleep);
      }

      time_of_flight++;

      if (tx_bytes != 0) {
        eventCount = 0;
      } else {
        eventCount++;
      }
    }
  }
}

}
