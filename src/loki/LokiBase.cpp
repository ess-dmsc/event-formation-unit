/** Copyright (C) 2019 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Implementation of the detector pipeline plugin for Loki
/// detectors.
//===----------------------------------------------------------------------===//

#include "LokiBase.h"

#include <cinttypes>
#include <common/EFUArgs.h>
#include <common/EV42Serializer.h>
#include <common/Producer.h>
#include <common/monitor/HistogramSerializer.h>
#include <common/RingBuffer.h>
#include <common/Trace.h>
#include <common/TimeString.h>
#include <common/TestImageUdder.h>

#include <unistd.h>

#include <common/SPSCFifo.h>
#include <common/Socket.h>
#include <common/TSCTimer.h>
#include <common/Timer.h>

#include <loki/geometry/Geometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Loki {

using namespace memory_sequential_consistent; // Lock free fifo

const char *classname = "Loki detector with ESS readout";

const int TSC_MHZ = 2900; // MJC's workstation - not reliable


LokiBase::LokiBase(BaseSettings const &settings, struct LokiSettings &LocalLokiSettings)
    : Detector("Loki", settings), LokiModuleSettings(LocalLokiSettings) {

  Stats.setPrefix(EFUSettings.GraphitePrefix, EFUSettings.GraphiteRegion);

  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off
  Stats.create("receive.packets", Counters.RxPackets);
  Stats.create("receive.bytes", Counters.RxBytes);
  Stats.create("receive.dropped", Counters.FifoPushErrors);
  Stats.create("receive.fifo_seq_errors", Counters.FifoSeqErrors);

  Stats.create("readouts.count", Counters.ReadoutsCount);
  Stats.create("readouts.error_bytes", Counters.ReadoutsErrorBytes);

  Stats.create("thread.processing_idle", Counters.RxIdle);

  Stats.create("events.count", Counters.Events);
  Stats.create("events.udder", Counters.EventsUdder);
  Stats.create("events.geometry_errors", Counters.GeometryErrors);

  Stats.create("transmit.bytes", Counters.TxBytes);

  /// \todo below stats are common to all detectors and could/should be moved
  Stats.create("kafka.produce_fails", Counters.kafka_produce_fails);
  Stats.create("kafka.ev_errors", Counters.kafka_ev_errors);
  Stats.create("kafka.ev_others", Counters.kafka_ev_others);
  Stats.create("kafka.dr_errors", Counters.kafka_dr_errors);
  Stats.create("kafka.dr_others", Counters.kafka_dr_noerrors);
  // clang-format on

  std::function<void()> inputFunc = [this]() { LokiBase::input_thread(); };
  Detector::AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    LokiBase::processing_thread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d Loki Rx ringbuffers of size %d",
         EthernetBufferMaxEntries, EthernetBufferSize);
  /// \todo the number 11 is a workaround
  EthernetRingbuffer = new RingBuffer<EthernetBufferSize>(EthernetBufferMaxEntries + 11);
  assert(EthernetRingbuffer != 0);
}

void LokiBase::input_thread() {
  /** Connection setup */
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(),
                         EFUSettings.DetectorPort);
  UDPReceiver receiver(local);
  // receiver.buflen(opts->buflen);
  receiver.setBufferSizes(0, EFUSettings.DetectorRxBufferSize);
  receiver.printBufferSizes();
  receiver.setRecvTimeout(0, 100000); /// secs, usecs 1/10s

  for (;;) {
    int rdsize;
    unsigned int eth_index = EthernetRingbuffer->getDataIndex();

    /** this is the processing step */
    EthernetRingbuffer->setDataLength(eth_index, 0);
    if ((rdsize = receiver.receive(EthernetRingbuffer->getDataBuffer(eth_index),
                                   EthernetRingbuffer->getMaxBufSize())) > 0) {
      EthernetRingbuffer->setDataLength(eth_index, rdsize);
      XTRACE(INPUT, DEB, "Received an udp packet of length %d bytes", rdsize);
      Counters.RxPackets++;
      Counters.RxBytes += rdsize;

      if (InputFifo.push(eth_index) == false) {
        Counters.FifoPushErrors++;
      } else {
        EthernetRingbuffer->getNextBuffer();
      }
    }

    // Checking for exit
    if (not runThreads) {
      XTRACE(INPUT, ALW, "Stopping input thread.");
      return;
    }
  }
}

void LokiBase::processing_thread() {
  const unsigned int NXTubes{8};
  const unsigned int NZTubes{4};
  const unsigned int NStraws{7};
  const unsigned int NYpos{512};
  Geometry geometry(NXTubes, NZTubes, NStraws, NYpos);

  EV42Serializer flatbuffer(KafkaBufferSize, "loki");
  Producer eventprod(EFUSettings.KafkaBroker, "LOKI_detector");
#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
  flatbuffer.setProducerCallback(
      std::bind(&Producer::produce2<uint8_t>, &eventprod, std::placeholders::_1));
#pragma GCC diagnostic pop

  if (EFUSettings.TestImage) {
    ESSGeometry essgeom(56, 512, 4, 1);
    XTRACE(PROCESS, ALW, "GENERATING TEST IMAGE!");
    Udder udder;
    udder.cachePixels(essgeom.nx(), essgeom.ny(), &essgeom);
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

      auto pixel_id = udder.getPixel(essgeom.nx(), essgeom.ny(), &essgeom);

      Counters.TxBytes += flatbuffer.addEvent(time_of_flight, pixel_id);
      Counters.EventsUdder++;

      if (EFUSettings.TestImageUSleep != 0) {
        usleep(EFUSettings.TestImageUSleep);
      }

      time_of_flight++;

      if (Counters.TxBytes != 0) {
        eventCount = 0;
      } else {
        eventCount++;
      }
    }
  }


  unsigned int data_index;
  TSCTimer produce_timer;
  Timer h5flushtimer;
  while (true) {
    if (InputFifo.pop(data_index)) { // There is data in the FIFO - do processing
      auto datalen = EthernetRingbuffer->getDataLength(data_index);
      if (datalen == 0) {
        Counters.FifoSeqErrors++;
        continue;
      }

      /// \todo use the Buffer<T> class here and in parser
      auto __attribute__((unused)) dataptr = EthernetRingbuffer->getDataBuffer(data_index);
      /// \todo add parser

      uint64_t efu_time = 1000000000LU * (uint64_t)time(NULL); // ns since 1970
      flatbuffer.pulseTime(efu_time);

      /// \todo traverse readouts
      //for (...) {
        // calculate strawid and ypos from four amplitudes
        // possibly add fpgaid to the stew
        auto tube = 0;
        auto straw = 0;
        auto ypos = 0;
        auto time = 0 ; // TOF in ns
        auto pixel_id = geometry.getPixelId(tube, straw, ypos);
        XTRACE(EVENT, DEB, "time: %u, tube %u, straw %u, ypos %u, pixel: %u",
               time, tube, straw, ypos, pixel_id);

        if (pixel_id == 0) {
          Counters.GeometryErrors++;
        } else {
          Counters.TxBytes += flatbuffer.addEvent(time, pixel_id);
          Counters.Events++;
        }
      //}

    } else {
      // There is NO data in the FIFO - do stop checks and sleep a little
      Counters.RxIdle++;
      usleep(10);
    }

    if (produce_timer.timetsc() >=
        EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ) {

      Counters.TxBytes += flatbuffer.produce();

      // if (!histograms.isEmpty()) {
      //   histfb.produce(histograms);
      //   histograms.clear();
      // }

      /// Kafka stats update - common to all detectors
      /// don't increment as producer keeps absolute count
      Counters.kafka_produce_fails = eventprod.stats.produce_fails;
      Counters.kafka_ev_errors = eventprod.stats.ev_errors;
      Counters.kafka_ev_others = eventprod.stats.ev_others;
      Counters.kafka_dr_errors = eventprod.stats.dr_errors;
      Counters.kafka_dr_noerrors = eventprod.stats.dr_noerrors;

      produce_timer.now();
    }

    if (not runThreads) {
      // \todo flush everything here
      XTRACE(INPUT, ALW, "Stopping processing thread.");
      return;
    }
  }
}

}
