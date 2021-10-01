// Copyright (C) 2019-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implementation of the detector pipeline plugin for Loki
/// detectors
//===----------------------------------------------------------------------===//

#include "LokiBase.h"

#include <cinttypes>
#include <common/EFUArgs.h>
#include <common/debug/Log.h>
#include <common/RuntimeStat.h>
#include <common/Socket.h>
#include <common/time/TSCTimer.h>
#include <common/TestImageUdder.h>
#include <common/time/TimeString.h>
#include <common/time/Timer.h>
#include <common/debug/Trace.h>
#include <common/monitor/HistogramSerializer.h>
#include <loki/LokiInstrument.h>
#include <stdio.h>
#include <unistd.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB
// #define ECDC_DEBUG_READOUT

namespace Loki {

const char *classname = "Loki detector with ESS readout";

LokiBase::LokiBase(BaseSettings const &Settings,
                   struct LokiSettings &LocalLokiSettings)
    : Detector("Loki", Settings), LokiModuleSettings(LocalLokiSettings) {

  Stats.setPrefix(EFUSettings.GraphitePrefix, EFUSettings.GraphiteRegion);

  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off
  Stats.create("receive.packets", Counters.RxPackets);
  Stats.create("receive.bytes", Counters.RxBytes);
  Stats.create("receive.dropped", Counters.FifoPushErrors);
  Stats.create("receive.fifo_seq_errors", Counters.FifoSeqErrors);

  // ESS Readout
  Stats.create("essheader.error_header", Counters.ErrorESSHeaders);
  Stats.create("essheader.error_buffer", Counters.ReadoutStats.ErrorBuffer);
  Stats.create("essheader.error_cookie", Counters.ReadoutStats.ErrorCookie);
  Stats.create("essheader.error_pad", Counters.ReadoutStats.ErrorPad);
  Stats.create("essheader.error_size", Counters.ReadoutStats.ErrorSize);
  Stats.create("essheader.error_version", Counters.ReadoutStats.ErrorVersion);
  Stats.create("essheader.error_output_queue", Counters.ReadoutStats.ErrorOutputQueue);
  Stats.create("essheader.error_type", Counters.ReadoutStats.ErrorTypeSubType);
  Stats.create("essheader.error_seqno", Counters.ReadoutStats.ErrorSeqNum);
  Stats.create("essheader.error_timehigh", Counters.ReadoutStats.ErrorTimeHigh);
  Stats.create("essheader.error_timefrac", Counters.ReadoutStats.ErrorTimeFrac);
  Stats.create("essheader.heartbeats", Counters.ReadoutStats.HeartBeats);

  // LoKI Readout Data
  Stats.create("readouts.headers", Counters.DataHeaders);
  Stats.create("readouts.count", Counters.Readouts);
  Stats.create("readouts.error_amplitude", Counters.ReadoutsBadAmpl);
  Stats.create("readouts.error_header", Counters.ErrorDataHeaders);
  Stats.create("readouts.error_bytes", Counters.ErrorBytes);
  Stats.create("readouts.tof_count", Counters.TofCount);
  Stats.create("readouts.tof_neg", Counters.TofNegative);
  Stats.create("readouts.prevtof_count", Counters.PrevTofCount);
  Stats.create("readouts.prevtof_neg", Counters.PrevTofNegative);

  // Logical and Digital geometry incl. Calibration
  Stats.create("geometry.ring_mapping_errors", Counters.RingErrors);
  Stats.create("geometry.fen_mapping_errors", Counters.FENErrors);
  Stats.create("geometry.calib_errors", Counters.CalibrationErrors);
  Stats.create("geometry.pos_low", Counters.ReadoutsClampLow);
  Stats.create("geometry.pos_high", Counters.ReadoutsClampHigh);

  // Events
  Stats.create("events.count", Counters.Events);
  Stats.create("events.pixel_errors", Counters.PixelErrors);
  Stats.create("events.outside_region", Counters.OutsideRegion);
  Stats.create("events.udder", Counters.EventsUdder);


  // System counters
  Stats.create("thread.input_idle", Counters.RxIdle);
  Stats.create("thread.processing_idle", Counters.ProcessingIdle);


  Stats.create("transmit.bytes", Counters.TxBytes);

  /// \todo below stats are common to all detectors and could/should be moved
  Stats.create("kafka.produce_fails", Counters.kafka_produce_fails);
  Stats.create("kafka.ev_errors", Counters.kafka_ev_errors);
  Stats.create("kafka.ev_others", Counters.kafka_ev_others);
  Stats.create("kafka.dr_errors", Counters.kafka_dr_errors);
  Stats.create("kafka.dr_others", Counters.kafka_dr_noerrors);
  // clang-format on

  std::function<void()> inputFunc = [this]() { LokiBase::inputThread(); };
  Detector::AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    LokiBase::processingThread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d Loki Rx ringbuffers of size %d",
         EthernetBufferMaxEntries, EthernetBufferSize);
}

void LokiBase::inputThread() {
  /** Connection setup */
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(),
                         EFUSettings.DetectorPort);

  UDPReceiver dataReceiver(local);
  dataReceiver.setBufferSizes(EFUSettings.TxSocketBufferSize,
                              EFUSettings.RxSocketBufferSize);
  dataReceiver.printBufferSizes();
  dataReceiver.setRecvTimeout(0, 100000); /// secs, usecs 1/10s

  while (runThreads) {
    int readSize;

    unsigned int rxBufferIndex = RxRingbuffer.getDataIndex();

    RxRingbuffer.setDataLength(rxBufferIndex, 0);

    if ((readSize = dataReceiver.receive(RxRingbuffer.getDataBuffer(rxBufferIndex),
                                         RxRingbuffer.getMaxBufSize())) > 0) {
      RxRingbuffer.setDataLength(rxBufferIndex, readSize);
      XTRACE(INPUT, DEB, "Received an udp packet of length %d bytes", readSize);
      Counters.RxPackets++;
      Counters.RxBytes += readSize;

      if (InputFifo.push(rxBufferIndex) == false) {
        Counters.FifoPushErrors++;
      } else {
        RxRingbuffer.getNextBuffer();
      }
    } else {
      Counters.RxIdle++;
    }
  }
  XTRACE(INPUT, ALW, "Stopping input thread.");
  return;
}

/// \brief Generate an Udder test image
/// \todo is probably not working after latest changes
void LokiBase::testImageUdder() {
  ESSGeometry LoKIGeometry(512, 224, 1, 1);
  XTRACE(PROCESS, ALW, "GENERATING TEST IMAGE!");
  Udder udderImage;
  udderImage.cachePixels(LoKIGeometry.nx(), LoKIGeometry.ny(), &LoKIGeometry);
  uint32_t TimeOfFlight = 0;
  while (runThreads) {
    static int EventCount = 0;
    if (EventCount == 0) {
      uint64_t EfuTime = 1000000000LU * (uint64_t)time(NULL); // ns since 1970
      Serializer->pulseTime(EfuTime);
    }

    auto pixelId = udderImage.getPixel(LoKIGeometry.nx(), LoKIGeometry.ny(), &LoKIGeometry);
    Counters.TxBytes += Serializer->addEvent(TimeOfFlight, pixelId);
    Counters.EventsUdder++;

    if (EFUSettings.TestImageUSleep != 0) {
      usleep(EFUSettings.TestImageUSleep);
    }

    TimeOfFlight++;

    if (Counters.TxBytes != 0) {
      EventCount = 0;
    } else {
      EventCount++;
    }
  }
  // \todo flush everything here
  XTRACE(INPUT, ALW, "Stopping processing thread.");
  return;
}

///
/// \brief Normal processing thread
void LokiBase::processingThread() {

  LokiInstrument Loki(Counters, LokiModuleSettings);

  Producer EventProducer(EFUSettings.KafkaBroker, "loki_detector");

  auto Produce = [&EventProducer](auto DataBuffer, auto Timestamp) {
    EventProducer.produce(DataBuffer, Timestamp);
  };

  Serializer = new EV42Serializer(KafkaBufferSize, "loki", Produce);
  Loki.setSerializer(Serializer); // would rather have this in LokiInstrument

  Producer EventProducerII(EFUSettings.KafkaBroker, "LOKI_debug");

  auto ProduceII = [&EventProducerII](auto DataBuffer, auto Timestamp) {
    EventProducerII.produce(DataBuffer, Timestamp);
  };

  SerializerII = new EV42Serializer(KafkaBufferSize, "loki", ProduceII);
  Loki.setSerializerII(SerializerII); // would rather have this in LokiInstrument

  if (EFUSettings.TestImage) {
    return testImageUdder();
  }

  unsigned int DataIndex;
  TSCTimer ProduceTimer, DebugTimer;

  RuntimeStat RtStat({Counters.RxPackets, Counters.Events, Counters.TxBytes});

  while (runThreads) {
    if (InputFifo.pop(DataIndex)) { // There is data in the FIFO - do processing
      auto DataLen = RxRingbuffer.getDataLength(DataIndex);
      if (DataLen == 0) {
        Counters.FifoSeqErrors++;
        continue;
      }

      /// \todo use the Buffer<T> class here and in parser?
      /// \todo avoid copying by passing reference to stats like for gdgem?
      auto DataPtr = RxRingbuffer.getDataBuffer(DataIndex);

      auto Res = Loki.ESSReadoutParser.validate(DataPtr, DataLen, ESSReadout::Parser::Loki4Amp);
      Counters.ReadoutStats = Loki.ESSReadoutParser.Stats;

      if (Res != ESSReadout::Parser::OK) {
        XTRACE(DATA, DEB, "Error parsing ESS readout header");
        Counters.ErrorESSHeaders++;
        continue;
      }

      // We have good header information, now parse readout data
      Res = Loki.LokiParser.parse(Loki.ESSReadoutParser.Packet.DataPtr,
                                  Loki.ESSReadoutParser.Packet.DataLength);

      Counters.TofCount = Loki.ESSReadoutParser.Packet.Time.Stats.TofCount;
      Counters.TofNegative = Loki.ESSReadoutParser.Packet.Time.Stats.TofNegative;
      Counters.PrevTofCount = Loki.ESSReadoutParser.Packet.Time.Stats.PrevTofCount;
      Counters.PrevTofNegative = Loki.ESSReadoutParser.Packet.Time.Stats.PrevTofNegative;

      // Process readouts, generate (end produce) events
      Loki.processReadouts();

    } else { // There is NO data in the FIFO - do stop checks and sleep a little
      Counters.ProcessingIdle++;
      usleep(10);
    }

#ifdef ECDC_DEBUG_READOUT
    if (DebugTimer.timetsc() >= 5ULL * 1000000 * TSC_MHZ) {
      printf("\nRING     |    FEN0     FEN1     FEN2     FEN3     FEN4     FEN5     FEN6     FEN7\n");
      printf("-----------------------------------------------------------------------------------\n");
      for (int ring = 0; ring < 8; ring++) {
        printf("ring %2d  | ", ring);
        for (int fen = 0; fen < 8; fen++) {
          printf("%8u ", Loki.LokiParser.HeaderCounters[ring][fen]);
        }
        printf("\n");
      }
      fflush(NULL);
      DebugTimer.reset();
    }
#endif

    if (ProduceTimer.timetsc() >= EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ) {

      RuntimeStatusMask = RtStat.getRuntimeStatusMask(
          {Counters.RxPackets, Counters.Events, Counters.TxBytes});

      Counters.TxBytes += Serializer->produce();
      SerializerII->produce();

      /// Kafka stats update - common to all detectors
      /// don't increment as producer keeps absolute count
      Counters.kafka_produce_fails = EventProducer.stats.produce_fails;
      Counters.kafka_ev_errors = EventProducer.stats.ev_errors;
      Counters.kafka_ev_others = EventProducer.stats.ev_others;
      Counters.kafka_dr_errors = EventProducer.stats.dr_errors;
      Counters.kafka_dr_noerrors = EventProducer.stats.dr_noerrors;

      ProduceTimer.reset();
    }
  }
  XTRACE(INPUT, ALW, "Stopping processing thread.");
  return;
}
} // namespace Loki
