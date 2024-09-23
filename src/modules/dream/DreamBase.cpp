// Copyright (C) 2016 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implementation of the detector pipeline plugin for DREAM
/// \todo unofficial - not reviewed Readout structure
//===----------------------------------------------------------------------===//

#include <common/RuntimeStat.h>
#include <common/debug/Trace.h>
#include <common/detector/EFUArgs.h>
#include <common/kafka/KafkaConfig.h>
#include <modules/dream/DreamBase.h>
#include <modules/dream/DreamInstrument.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Dream {

const char *classname = "DREAM detector with ESS readout";

DreamBase::DreamBase(BaseSettings const &Settings) : Detector(Settings) {

  Stats.setPrefix(EFUSettings.GraphitePrefix, EFUSettings.GraphiteRegion);

  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off
  Stats.create("receive.packets", ITCounters.RxPackets);
  Stats.create("receive.bytes", ITCounters.RxBytes);
  Stats.create("receive.dropped", ITCounters.FifoPushErrors);
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
  Stats.create("essheader.version.v0", Counters.ReadoutStats.Version0Header);
  Stats.create("essheader.version.v1", Counters.ReadoutStats.Version1Header);

  // ESS Readout Data Header
  Stats.create("readouts.count", Counters.Readouts);
  Stats.create("readouts.headers", Counters.DataHeaders);
  Stats.create("readouts.error_buffer", Counters.BufferErrors);
  Stats.create("readouts.error_datalen", Counters.DataLenErrors);
  Stats.create("readouts.error_fiber", Counters.FiberErrors);
  Stats.create("readouts.error_fen", Counters.FENErrors);

  // Config related
  Stats.create("readouts.error_ringmapping", Counters.RingMappingErrors);
  Stats.create("readouts.error_fen_mapping", Counters.FENMappingErrors);
  Stats.create("readouts.error_config", Counters.ConfigErrors);

  //
  Stats.create("thread.input_idle", ITCounters.RxIdle);
  Stats.create("thread.processing_idle", Counters.ProcessingIdle);

  Stats.create("events.count", Counters.Events);
  Stats.create("events.geometry_errors", Counters.GeometryErrors);

  Stats.create("transmit.monitor_packets", Counters.TxRawReadoutPackets);
  Stats.create("transmit.calibmode_packets", ITCounters.CalibModePackets);

  // Produce cause call stats
  Stats.create("produce.cause.timeout", Counters.ProduceCauseTimeout);

  /// \todo below stats are common to all detectors and could/should be moved
  Stats.create("kafka.config_errors", Counters.KafkaStats.config_errors);
  Stats.create("kafka.produce_bytes_ok", Counters.KafkaStats.produce_bytes_ok);
  Stats.create("kafka.produce_bytes_error", Counters.KafkaStats.produce_bytes_error);
  Stats.create("kafka.produce_calls", Counters.KafkaStats.produce_calls);
  Stats.create("kafka.produce_no_errors", Counters.KafkaStats.produce_no_errors);
  Stats.create("kafka.produce_errors", Counters.KafkaStats.produce_errors);
  Stats.create("kafka.err_unknown_topic", Counters.KafkaStats.err_unknown_topic);
  Stats.create("kafka.err_queue_full", Counters.KafkaStats.err_queue_full);
  Stats.create("kafka.err_other", Counters.KafkaStats.err_other);
  Stats.create("kafka.ev_errors", Counters.KafkaStats.ev_errors);
  Stats.create("kafka.ev_others", Counters.KafkaStats.ev_others);
  Stats.create("kafka.dr_errors", Counters.KafkaStats.dr_errors);
  Stats.create("kafka.dr_others", Counters.KafkaStats.dr_noerrors);
  Stats.create("kafka.librdkafka_msg_cnt", Counters.KafkaStats.librdkafka_msg_cnt);
  Stats.create("kafka.librdkafka_msg_size", Counters.KafkaStats.librdkafka_msg_size);
  // clang-format on
  std::function<void()> inputFunc = [this]() { inputThread(); };
  AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    DreamBase::processingThread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d Dream Rx ringbuffers of size %d",
         EthernetBufferMaxEntries, EthernetBufferSize);
}

///
/// \brief Normal processing thread
void DreamBase::processingThread() {

  DreamInstrument Dream(Counters, EFUSettings);

  KafkaConfig KafkaCfg(EFUSettings.KafkaConfigFile);

  Producer EventProducer(EFUSettings.KafkaBroker, EFUSettings.KafkaTopic,
                         KafkaCfg.CfgParms);

  auto Produce = [&EventProducer](auto DataBuffer, auto Timestamp) {
    EventProducer.produce(DataBuffer, Timestamp);
  };

  Producer MonitorProducer(EFUSettings.KafkaBroker, EFUSettings.KafkaDebugTopic,
                           KafkaCfg.CfgParms);
  auto ProduceMonitor = [&MonitorProducer](auto DataBuffer, auto Timestamp) {
    MonitorProducer.produce(DataBuffer, Timestamp);
  };

  MonitorSerializer = new AR51Serializer("dream", ProduceMonitor);

  Serializer =
      new EV44Serializer(KafkaBufferSize, EFUSettings.DetectorName, Produce);

  Stats.create("produce.cause.pulse_change",
               Serializer->stats().ProduceRefTimeTriggered);
  Stats.create("produce.cause.max_events_reached",
               Serializer->stats().ProduceTriggeredMaxEvents);

  Dream.setSerializer(Serializer); // would rather have this in DreamInstrument

  unsigned int DataIndex;
  TSCTimer ProduceTimer;

  RuntimeStat RtStat({ITCounters.RxPackets, Counters.Events,
                      Counters.KafkaStats.produce_bytes_ok});

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

      auto Res = Dream.ESSReadoutParser.validate(DataPtr, DataLen, Dream.Type);
      Counters.ReadoutStats = Dream.ESSReadoutParser.Stats;

      if (Res != ESSReadout::Parser::OK) {
        XTRACE(DATA, DEB, "Error parsing ESS readout header");
        Counters.ErrorESSHeaders++;
        continue;
      }

      // We have good header information, now parse readout data
      Res = Dream.DreamParser.parse(Dream.ESSReadoutParser.Packet.DataPtr,
                                    Dream.ESSReadoutParser.Packet.DataLength);

      // Process readouts, generate (end produce) events
      Dream.processReadouts();

      // send monitoring data
      if (ITCounters.RxPackets % EFUSettings.MonitorPeriod <
          EFUSettings.MonitorSamples) {
        XTRACE(PROCESS, DEB, "Serialize and stream monitor data for packet %lu",
               ITCounters.RxPackets);
        MonitorSerializer->serialize((uint8_t *)DataPtr, DataLen);
        MonitorSerializer->produce();
        Counters.TxRawReadoutPackets++;
      }

    } else { // There is NO data in the FIFO - do stop checks and sleep a little
      Counters.ProcessingIdle++;
      usleep(10);
    }

    if (ProduceTimer.timetsc() >=
        EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ) {

      RuntimeStatusMask =
          RtStat.getRuntimeStatusMask({ITCounters.RxPackets, Counters.Events,
                                       Counters.KafkaStats.produce_bytes_ok});

      Serializer->produce();
      Counters.ProduceCauseTimeout++;

      /// Kafka stats update - common to all detectors
      /// don't increment as producer keeps absolute count
      Counters.KafkaStats = EventProducer.stats;

      ProduceTimer.reset();
    }
  }
  XTRACE(INPUT, ALW, "Stopping processing thread.");
  return;
}
} // namespace Dream
