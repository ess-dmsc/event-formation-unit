// Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief A pixel generator for performance testing (delete?)
/// \todo delete?
///
//===----------------------------------------------------------------------===//

#include "PerfGenBase.h"

#include <cinttypes>
#include <common/TestImageUdder.h>
#include <common/debug/Trace.h>
#include <common/detector/EFUArgs.h>
#include <common/kafka/EV42Serializer.h>
#include <common/kafka/Producer.h>
#include <common/time/TimeString.h>

#include <unistd.h>

#include <common/memory/SPSCFifo.h>
#include <common/system/Socket.h>
#include <common/time/TSCTimer.h>
#include <common/time/Timer.h>

#include <logical_geometry/ESSGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace PerfGen {

const char *classname = "PerfGen Pixel Generator";

PerfGenBase::PerfGenBase(BaseSettings const &settings,
                         struct PerfGenSettings &LocalPerfGenSettings)
    : Detector("PerfGen", settings), PerfGenSettings(LocalPerfGenSettings) {

  Stats.setPrefix(EFUSettings.GraphitePrefix, EFUSettings.GraphiteRegion);

  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off
  Stats.create("events.udder", mystats.events_udder);
  Stats.create("transmit.bytes", mystats.tx_bytes);

  /// \todo below stats are common to all detectors and could/should be moved
  Stats.create("kafka.produce_calls", mystats.kafka_produce_calls);
  Stats.create("kafka.produce_fails", mystats.kafka_produce_fails);
  Stats.create("kafka.ev_errors", mystats.kafka_ev_errors);
  Stats.create("kafka.ev_others", mystats.kafka_ev_others);
  Stats.create("kafka.dr_errors", mystats.kafka_dr_errors);
  Stats.create("kafka.dr_others", mystats.kafka_dr_noerrors);
  // clang-format on

  std::function<void()> processingFunc = [this]() {
    PerfGenBase::processingThread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");
}

void PerfGenBase::processingThread() {

  if (EFUSettings.KafkaTopic == "") {
    EFUSettings.KafkaTopic = "perfgen_detector";
  }

  Producer EventProducer(EFUSettings.KafkaBroker, EFUSettings.KafkaTopic);

  auto Produce = [&EventProducer](auto DataBuffer, auto Timestamp) {
    EventProducer.produce(DataBuffer, Timestamp);
  };

  EV42Serializer Serializer(kafka_buffer_size, "perfgen", Produce);

  ESSGeometry ESSGeom(64, 64, 1, 1);

  XTRACE(PROCESS, ALW, "GENERATING TEST IMAGE!");
  Udder UdderImage;
  UdderImage.cachePixels(ESSGeom.nx(), ESSGeom.ny(), &ESSGeom);

  int TimeOfFlight{0};
  int EventsPerPulse{500};

  // ns since 1970 - but with a resolution of one second
  uint64_t EfuTimeRef = 1000000000LU * (uint64_t)time(NULL);
  Timer Elapsed; // provide a us timer

  while (runThreads) {
    // ns since 1970 - but with us resolution
    uint64_t EfuTime = EfuTimeRef + 1000 * Elapsed.timeus();
    XTRACE(DATA, DEB, "EFU Time (ns since 1970): %lu", EfuTime);
    Serializer.pulseTime(EfuTime);

    for (int i = 0; i < EventsPerPulse; i++) {
      auto PixelId = UdderImage.getPixel(ESSGeom.nx(), ESSGeom.ny(), &ESSGeom);
      Serializer.addEvent(TimeOfFlight, PixelId);
      mystats.events_udder++;
      TimeOfFlight++;
    }
    //Serializer.checkAndSetPulseTime(EfuTime);


    usleep(EFUSettings.TestImageUSleep);

    /// Kafka stats update - common to all detectors
    /// don't increment as Producer & Serializer keep absolute count
    mystats.kafka_produce_fails = EventProducer.stats.produce_fails;
    mystats.kafka_ev_errors = EventProducer.stats.ev_errors;
    mystats.kafka_ev_others = EventProducer.stats.ev_others;
    mystats.kafka_dr_errors = EventProducer.stats.dr_errors;
    mystats.kafka_dr_noerrors = EventProducer.stats.dr_noerrors;
    mystats.tx_bytes = Serializer.TxBytes;
    TimeOfFlight = 0;
  }
  // \todo flush everything here
  XTRACE(INPUT, ALW, "Stopping processing thread.");
  return;
}

} // namespace PerfGen
