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

const char *classname = "PerfGen Pixel Generator";

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
    PerfGenBase::processingThread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");
}

void PerfGenBase::processingThread() {

  if (EFUSettings.KafkaTopic == "") {
    EFUSettings.KafkaTopic = "PERFGEN_detector";
  }

  Producer EventProducer(EFUSettings.KafkaBroker, EFUSettings.KafkaTopic);

  auto Produce = [&EventProducer](auto DataBuffer, auto Timestamp) {
    EventProducer.produce(DataBuffer, Timestamp);
  };

  EV42Serializer FlatBuffer(kafka_buffer_size, "multiblade", Produce);

  ESSGeometry ESSGeom(64, 64, 1, 1);

  XTRACE(PROCESS, ALW, "GENERATING TEST IMAGE!");
  Udder UdderImage;
  UdderImage.cachePixels(ESSGeom.nx(), ESSGeom.ny(), &ESSGeom);
  uint32_t TimeOfFlight = 0;
  while (runThreads) {
    static int EventCount = 0;
    if (EventCount == 0) {
      uint64_t EfuTime = 1000000000LU * (uint64_t)time(NULL); // ns since 1970
      FlatBuffer.pulseTime(EfuTime);
    }

    auto PixelId = UdderImage.getPixel(ESSGeom.nx(), ESSGeom.ny(), &ESSGeom);
    auto TxBytes = FlatBuffer.addEvent(TimeOfFlight, PixelId);
    mystats.tx_bytes += TxBytes;
    mystats.events_udder++;

    if (EFUSettings.TestImageUSleep != 0) {
      usleep(EFUSettings.TestImageUSleep);
    }

    TimeOfFlight++;

    if (TxBytes != 0) {
      EventCount = 0;
    } else {
      EventCount++;
    }
  }
  // \todo flush everything here
  XTRACE(INPUT, ALW, "Stopping processing thread.");
  return;
}

}
