// Copyright (C) 2021 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implementation of the detector pipeline plugin for DREAM
/// \todo unofficial - not reviewed Readout structure
//===----------------------------------------------------------------------===//

#include "DreamBase.h"

#include <cinttypes>
#include <common/detector/EFUArgs.h>
#include <common/debug/Log.h>
#include <common/RuntimeStat.h>
#include <common/system/Socket.h>
#include <common/time/TSCTimer.h>
#include <common/TestImageUdder.h>
#include <common/time/TimeString.h>
#include <common/time/Timer.h>
#include <common/debug/Trace.h>
#include <dream/DreamInstrument.h>
#include <stdio.h>
#include <unistd.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Dream {

const char *classname = "DREAM detector with ESS readout";

DreamBase::DreamBase(BaseSettings const &Settings,
                     struct DreamSettings &LocalDreamSettings)
    : Detector("Dream", Settings), DreamModuleSettings(LocalDreamSettings) {

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

  // ESS Readout Data Header
  Stats.create("readouts.count", Counters.Readouts);
  Stats.create("readouts.headers", Counters.Headers);
  Stats.create("readouts.error_bytes", Counters.ErrorBytes);
  Stats.create("readouts.error_header", Counters.ErrorHeaders);



  //
  Stats.create("thread.input_idle", Counters.RxIdle);
  Stats.create("thread.processing_idle", Counters.ProcessingIdle);

  Stats.create("events.count", Counters.Events);

  Stats.create("events.mapping_errors", Counters.MappingErrors);
  Stats.create("events.geometry_errors", Counters.GeometryErrors);

  Stats.create("transmit.bytes", Counters.TxBytes);

  /// \todo below stats are common to all detectors and could/should be moved
  Stats.create("kafka.produce_fails", Counters.kafka_produce_fails);
  Stats.create("kafka.ev_errors", Counters.kafka_ev_errors);
  Stats.create("kafka.ev_others", Counters.kafka_ev_others);
  Stats.create("kafka.dr_errors", Counters.kafka_dr_errors);
  Stats.create("kafka.dr_others", Counters.kafka_dr_noerrors);
  // clang-format on

  std::function<void()> inputFunc = [this]() { DreamBase::inputThread(); };
  Detector::AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    DreamBase::processingThread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d Dream Rx ringbuffers of size %d",
         EthernetBufferMaxEntries, EthernetBufferSize);
}

void DreamBase::inputThread() {
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

///
/// \brief Normal processing thread
void DreamBase::processingThread() {

  DreamInstrument Dream(Counters, DreamModuleSettings);

  Producer EventProducer(EFUSettings.KafkaBroker, "dream_detector");

  auto Produce = [&EventProducer](auto DataBuffer, auto Timestamp) {
    EventProducer.produce(DataBuffer, Timestamp);
  };

  Serializer = new EV42Serializer(KafkaBufferSize, "dream", Produce);
  Dream.setSerializer(Serializer); // would rather have this in DreamInstrument

  unsigned int DataIndex;
  TSCTimer ProduceTimer;

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

      auto Res = Dream.ESSReadoutParser.validate(DataPtr, DataLen, ESSReadout::Parser::DREAM);
      Counters.ReadoutStats = Dream.ESSReadoutParser.Stats;

      if (Res != ESSReadout::Parser::OK) {
        XTRACE(DATA, DEB, "Error parsing ESS readout header");
        Counters.ErrorHeaders++;
        continue;
      }

      // We have good header information, now parse readout data
      Res = Dream.DreamParser.parse(Dream.ESSReadoutParser.Packet.DataPtr,
                                    Dream.ESSReadoutParser.Packet.DataLength);

      // Process readouts, generate (end produce) events
      Dream.processReadouts();

    } else { // There is NO data in the FIFO - do stop checks and sleep a little
      Counters.ProcessingIdle++;
      usleep(10);
    }

    if (ProduceTimer.timetsc() >=
        EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ) {

      RuntimeStatusMask = RtStat.getRuntimeStatusMask(
          {Counters.RxPackets, Counters.Events, Counters.TxBytes});

      Counters.TxBytes += Serializer->produce();

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
} // namespace Dream
