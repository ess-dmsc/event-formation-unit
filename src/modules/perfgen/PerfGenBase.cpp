// Copyright (C) 2020 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief A pixel generator for performance testing
///
//===----------------------------------------------------------------------===//

#include "PerfGenBase.h"

#include <cinttypes>
#include <common/TestImageUdder.h>
#include <common/debug/Trace.h>
#include <common/detector/EFUArgs.h>
#include <common/kafka/EV44Serializer.h>
#include <common/kafka/KafkaConfig.h>
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

PerfGenBase::PerfGenBase(BaseSettings const &settings) : Detector(settings) {

  Stats.setPrefix(EFUSettings.GraphitePrefix, EFUSettings.GraphiteRegion);

  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off
  Stats.create("events.udder", Counters.events_udder);
  // clang-format on

  std::function<void()> processingFunc = [this]() {
    PerfGenBase::processingThread();
  };
  Detector::AddThreadFunction(processingFunc, "generator");
}

void PerfGenBase::processingThread() {

  if (EFUSettings.KafkaTopic == "") {
    EFUSettings.KafkaTopic = "perfgen_detector";
  }

  KafkaConfig KafkaCfg(EFUSettings.KafkaConfigFile);
  Producer EventProducer(EFUSettings.KafkaBroker, EFUSettings.KafkaTopic,
                         KafkaCfg.CfgParms, &Stats);

  auto Produce = [&EventProducer](auto DataBuffer, auto Timestamp) {
    EventProducer.produce(DataBuffer, Timestamp);
  };

  EV44Serializer Serializer(kafka_buffer_size, "perfgen", Produce);

  ESSGeometry ESSGeom(64, 64, 1, 1);

  XTRACE(PROCESS, ALW, "GENERATING TEST IMAGE!");
  Udder UdderImage;
  UdderImage.cachePixels(ESSGeom.nx(), ESSGeom.ny(), &ESSGeom);

  int TimeOfFlight{0};
  uint32_t EventsPerPulse{EFUSettings.TestImageEventsPerPulse};

  // ns since 1970 - but with a resolution of one second
  uint64_t EfuTimeRef = 1000000000LU * (uint64_t)time(NULL);
  Timer Elapsed; // provide a us timer

  while (runThreads) {
    // ns since 1970 - but with us resolution
    uint64_t EfuTime = EfuTimeRef + 1000 * Elapsed.timeus();
    XTRACE(DATA, DEB, "EFU Time (ns since 1970): %lu", EfuTime);
    Serializer.checkAndSetReferenceTime(EfuTime);

    for (uint32_t i = 0; i < EventsPerPulse; i++) {
      auto PixelId = UdderImage.getPixel(ESSGeom.nx(), ESSGeom.ny(), &ESSGeom);
      Serializer.addEvent(TimeOfFlight, PixelId);
      Counters.events_udder++;
      TimeOfFlight++;
    }

    usleep(EFUSettings.TestImageUSleep);

    EventProducer.poll(0);

    TimeOfFlight = 0;
  }
  /// \todo flush everything here
  XTRACE(INPUT, ALW, "Stopping generator thread.");
  return;
}

} // namespace PerfGen
