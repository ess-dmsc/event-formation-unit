// Copyright (C) 2019-2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implementation of the detector pipeline plugin for Caen
/// detectors
//===----------------------------------------------------------------------===//

#include "caen/CaenBase.h"

#include <caen/CaenInstrument.h>
#include <cinttypes>
#include <common/RuntimeStat.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/detector/EFUArgs.h>
#include <common/kafka/KafkaConfig.h>
#include <common/system/Socket.h>
#include <common/time/TSCTimer.h>
#include <common/time/TimeString.h>
#include <common/time/Timer.h>
#include <stdio.h>
#include <unistd.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

const char *classname = "Caen detector with ESS readout";

CaenBase::CaenBase(BaseSettings const &settings) : Detector(settings) {

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
  Stats.create("readouts.tof_high", Counters.TofHigh);
  Stats.create("readouts.prevtof_high", Counters.PrevTofHigh);

  // Logical and Digital geometry incl. Calibration
  Stats.create("geometry.ring_mapping_errors", Counters.RingErrors);
  Stats.create("geometry.fen_mapping_errors", Counters.FENErrors);
  Stats.create("geometry.calib_errors", Counters.CalibrationErrors);
  Stats.create("geometry.pos_low", Counters.ReadoutsClampLow);
  Stats.create("geometry.pos_high", Counters.ReadoutsClampHigh);

  // Events
  Stats.create("events.count", Counters.Events);
  Stats.create("events.pixel_errors", Counters.PixelErrors);

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

  std::function<void()> inputFunc = [this]() { CaenBase::inputThread(); };
  Detector::AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    CaenBase::processingThread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d Caen Rx ringbuffers of size %d",
         EthernetBufferMaxEntries, EthernetBufferSize);
}

void CaenBase::inputThread() {
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
    if ((readSize =
             dataReceiver.receive(RxRingbuffer.getDataBuffer(rxBufferIndex),
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
void CaenBase::processingThread() {

  if (EFUSettings.KafkaTopic == "") {
    XTRACE(INIT, ALW, "Setting default Kafka topic to loki_detector");
    EFUSettings.KafkaTopic = "loki_detector";
  }

  KafkaConfig KafkaCfg(EFUSettings.KafkaConfigFile);
  Producer EventProducer(EFUSettings.KafkaBroker, EFUSettings.KafkaTopic,
                         KafkaCfg.CfgParms);

  auto Produce = [&EventProducer](auto DataBuffer, auto Timestamp) {
    EventProducer.produce(DataBuffer, Timestamp);
  };

  Serializer = new EV42Serializer(KafkaBufferSize, "caen", Produce);
  CaenInstrument Caen(Counters, EFUSettings);
  Caen.setSerializer(Serializer); // would rather have this in CaenInstrument

  Producer EventProducerII(EFUSettings.KafkaBroker, "CAEN_debug",
                           KafkaCfg.CfgParms);

  auto ProduceII = [&EventProducerII](auto DataBuffer, auto Timestamp) {
    EventProducerII.produce(DataBuffer, Timestamp);
  };

  SerializerII = new EV42Serializer(KafkaBufferSize, "caen", ProduceII);
  Caen.setSerializerII(
      SerializerII); // would rather have this in CaenInstrument

  unsigned int DataIndex;
  TSCTimer ProduceTimer(EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ);

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

      auto Res = Caen.ESSReadoutParser.validate(DataPtr, DataLen,
                                                ESSReadout::Parser::LOKI);
      Counters.ReadoutStats = Caen.ESSReadoutParser.Stats;

      if (Res != ESSReadout::Parser::OK) {
        XTRACE(DATA, DEB, "Error parsing ESS readout header");
        Counters.ErrorESSHeaders++;
        continue;
      }

      // We have good header information, now parse readout data
      Res = Caen.CaenParser.parse(Caen.ESSReadoutParser.Packet.DataPtr,
                                  Caen.ESSReadoutParser.Packet.DataLength);

      // Process readouts, generate (end produce) events
      Caen.processReadouts();

      Counters.TofCount = Caen.ESSReadoutParser.Packet.Time.Stats.TofCount;
      Counters.TofNegative =
          Caen.ESSReadoutParser.Packet.Time.Stats.TofNegative;
      Counters.PrevTofCount =
          Caen.ESSReadoutParser.Packet.Time.Stats.PrevTofCount;
      Counters.PrevTofNegative =
          Caen.ESSReadoutParser.Packet.Time.Stats.PrevTofNegative;
      Counters.TofHigh = Caen.ESSReadoutParser.Packet.Time.Stats.TofHigh;
      Counters.PrevTofHigh =
          Caen.ESSReadoutParser.Packet.Time.Stats.PrevTofHigh;

    } else { // There is NO data in the FIFO - do stop checks and sleep a little
      Counters.ProcessingIdle++;
      usleep(10);
    }

    if (ProduceTimer.timeout()) {
      // XTRACE(DATA, DEB, "Serializer timer timed out, producing message now");
      RuntimeStatusMask = RtStat.getRuntimeStatusMask(
          {Counters.RxPackets, Counters.Events, Counters.TxBytes});

      Serializer->produce();
      SerializerII->produce();
    }
    /// Kafka stats update - common to all detectors
    /// don't increment as Producer & Serializer keep absolute count
    Counters.kafka_produce_fails = EventProducer.stats.produce_fails;
    Counters.kafka_ev_errors = EventProducer.stats.ev_errors;
    Counters.kafka_ev_others = EventProducer.stats.ev_others;
    Counters.kafka_dr_errors = EventProducer.stats.dr_errors;
    Counters.kafka_dr_noerrors = EventProducer.stats.dr_noerrors;
    Counters.TxBytes = Serializer->TxBytes;
  }
  XTRACE(INPUT, ALW, "Stopping processing thread.");
  return;
}
} // namespace Caen
