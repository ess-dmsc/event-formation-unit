// Copyright (C) 2022 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CBM instrument base class which define the detector and required data
///        processing
///
//===----------------------------------------------------------------------===//

#include <common/RuntimeStat.h>
#include <common/kafka/EV44Serializer.h>
#include <common/kafka/KafkaConfig.h>
#include <common/memory/HashMap2D.h>
#include <memory>
#include <modules/cbm/CbmBase.h>
#include <modules/cbm/CbmInstrument.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace cbm {

using namespace fbserializer;

CbmBase::CbmBase(BaseSettings const &settings)
    : Detector(settings), CbmConfiguration(EFUSettings.ConfigFile) {

  Stats.setPrefix(EFUSettings.GraphitePrefix, EFUSettings.GraphiteRegion);

  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off

  // Rx and Tx stats
  Stats.create("receive.packets", ITCounters.RxPackets);
  Stats.create("receive.bytes", ITCounters.RxBytes);
  Stats.create("receive.dropped", ITCounters.FifoPushErrors);
  Stats.create("receive.fifo_seq_errors", Counters.FifoSeqErrors);

  // ESS Readout header stats
  Stats.create("essheader.errors.header", Counters.ErrorESSHeaders);
  Stats.create("essheader.errors.buffer", Counters.ReadoutStats.ErrorBuffer);
  Stats.create("essheader.errors.cookie", Counters.ReadoutStats.ErrorCookie);
  Stats.create("essheader.errors.pad", Counters.ReadoutStats.ErrorPad);
  Stats.create("essheader.errors.size", Counters.ReadoutStats.ErrorSize);
  Stats.create("essheader.errors.version", Counters.ReadoutStats.ErrorVersion);
  Stats.create("essheader.errors.output_queue", Counters.ReadoutStats.ErrorOutputQueue);
  Stats.create("essheader.errors.type", Counters.ReadoutStats.ErrorTypeSubType);
  Stats.create("essheader.errors.seqno", Counters.ReadoutStats.ErrorSeqNum);
  Stats.create("essheader.errors.timehigh", Counters.ReadoutStats.ErrorTimeHigh);
  Stats.create("essheader.errors.timefrac", Counters.ReadoutStats.ErrorTimeFrac);
  Stats.create("essheader.heartbeats", Counters.ReadoutStats.HeartBeats);
  Stats.create("essheader.version.v0", Counters.ReadoutStats.Version0Header);
  Stats.create("essheader.version.v1", Counters.ReadoutStats.Version1Header);

  Stats.create("parsing.readout_parsed", Counters.CbmStats.Readouts);

  // Readout parsing errors - readout dropped
  Stats.create("parsing.errors.size", Counters.CbmStats.ErrorSize);
  Stats.create("parsing.errors.fiber", Counters.CbmStats.ErrorFiber);
  Stats.create("parsing.errors.fen", Counters.CbmStats.ErrorFEN);
  Stats.create("parsing.errors.type", Counters.CbmStats.ErrorType);
  Stats.create("parsing.errors.adc", Counters.CbmStats.ErrorADC);
  Stats.create("parsing.errors.datalen", Counters.CbmStats.ErrorDataLength);
  Stats.create("parsing.errors.timefrac", Counters.CbmStats.ErrorTimeFrac);
  Stats.create("parsing.errors.no_data", Counters.CbmStats.NoData);

  // Readout processing stats
  Stats.create("readouts.processed", Counters.CbmCounts);
  Stats.create("readouts.type.ttl_proccessed", Counters.TTLReadoutsProcessed);
  Stats.create("readouts.type.ibm_processed", Counters.IBMReadoutsProcessed);

  // Events published
  Stats.create("events.ibm", Counters.IBMEvents);
  Stats.create("events.ttl", Counters.TTLEvents);

  // Readout processing errors - readout dropped
  Stats.create("readouts.errors.ring_mismatch", Counters.RingCfgError);
  Stats.create("readouts.errors.no_serializer", Counters.NoSerializerCfgError);
  Stats.create("readouts.errors.type", Counters.TypeNotSupported);
  Stats.create("readouts.errors.time", Counters.TimeError);

  // Time stats
  Stats.create("readouts.tof_count", Counters.TimeStats.TofCount);
  Stats.create("readouts.tof_neg", Counters.TimeStats.TofNegative);
  Stats.create("readouts.prevtof_count", Counters.TimeStats.PrevTofCount);
  Stats.create("readouts.prevtof_neg", Counters.TimeStats.PrevTofNegative);

  //
  Stats.create("thread.receive_idle", ITCounters.RxIdle);
  Stats.create("thread.processing_idle", Counters.ProcessingIdle);

  // Produce cause call stats
  Stats.create("produce.cause.timeout", Counters.ProduceCauseTimeout);

  /// \todo below stats are common to all detectors and could/should be moved
  Stats.create("kafka.config_errors", Counters.ProducerStats.config_errors);
  Stats.create("kafka.produce_bytes_ok", Counters.ProducerStats.produce_bytes_ok);
  Stats.create("kafka.produce_bytes_error", Counters.ProducerStats.produce_bytes_error);
  Stats.create("kafka.produce_calls", Counters.ProducerStats.produce_calls);
  Stats.create("kafka.produce_no_errors", Counters.ProducerStats.produce_no_errors);
  Stats.create("kafka.produce_errors", Counters.ProducerStats.produce_errors);
  
  Stats.create("kafka.brokers.tx_bytes", Counters.ProducerStats.BytesTransmittedToBrokers);
  Stats.create("kafka.brokers.tx_req_retries", Counters.ProducerStats.TxRequestRetries);

  Stats.create("kafka.msg.num_of_msg_in_queue", Counters.ProducerStats.NumberOfMsgInQueue);
  Stats.create("kafka.msg.max_num_of_msg_in_queue", Counters.ProducerStats.MaxNumOfMsgInQueue);
  Stats.create("kafka.msg.bytes_of_msg_in_queue", Counters.ProducerStats.BytesOfMsgInQueue);
  Stats.create("kafka.msg.max_bytes_of_msg_in_queue", Counters.ProducerStats.MaxBytesOfMsgInQueue);
  Stats.create("kafka.msg.delivery_success", Counters.ProducerStats.MsgDeliverySuccess);
  Stats.create("kafka.msg.status_persisted", Counters.ProducerStats.MsgStatusPersisted);
  Stats.create("kafka.msg.status_not_persisted", Counters.ProducerStats.MsgStatusNotPersisted);
  Stats.create("kafka.msg.status_possibly_persisted", Counters.ProducerStats.MsgStatusPossiblyPersisted);
  Stats.create("kafka.msg.msg_delivery_event", Counters.ProducerStats.TotalMsgDeliveryEvent);

  /// Kafka error stats
  Stats.create("kafka.error.msg_delivery", Counters.ProducerStats.MsgError);
  Stats.create("kafka.error.transmission", Counters.ProducerStats.TransmissionErrors);
  Stats.create("kafka.error.unknown_topic", Counters.ProducerStats.ErrUnknownTopic);
  Stats.create("kafka.error.queue_full", Counters.ProducerStats.ErrQueueFull);
  Stats.create("kafka.error.timeout", Counters.ProducerStats.ErrTimeout);
  Stats.create("kafka.error.msg_timeout", Counters.ProducerStats.ErrMsgTimeout);
  Stats.create("kafka.error.transport", Counters.ProducerStats.ErrTransport);
  Stats.create("kafka.error.authentication", Counters.ProducerStats.AuthError);
  Stats.create("kafka.error.other", Counters.ProducerStats.ErrOther);

  // clang-format on
  std::function<void()> inputFunc = [this]() { inputThread(); };
  AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    CbmBase::processing_thread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d CBM Rx ringbuffers of size %d",
         EthernetBufferMaxEntries, EthernetBufferSize);
}

void CbmBase::processing_thread() {
  /// \todo: properly the quite and stop application and log the fault.
  if (EFUSettings.KafkaTopic == "") {
    XTRACE(INPUT, ALW, "Missing topic - mandatory for beam monitor");
    stopThreads();
  }
  XTRACE(INPUT, ALW, "Kafka topic %s", EFUSettings.KafkaTopic.c_str());

  // Event producer
  KafkaConfig KafkaCfg(EFUSettings.KafkaConfigFile);
  Producer eventprod(EFUSettings.KafkaBroker, EFUSettings.KafkaTopic,
                     KafkaCfg.CfgParms);

  auto Produce = [&eventprod](const auto &DataBuffer, const auto &Timestamp) {
    eventprod.produce(DataBuffer, Timestamp);
  };

  // Process instrument config file
  XTRACE(INIT, ALW, "Loading configuration file %s",
         EFUSettings.ConfigFile.c_str());

  CbmConfiguration.loadAndApply();

  // Create serializers
  EV44SerializerMapPtr.reset(
      new HashMap2D<EV44Serializer>(CbmConfiguration.Parms.NumOfFENs));
  HistogramSerializerMapPtr.reset(new HashMap2D<HistogramSerializer<int32_t>>(
      CbmConfiguration.Parms.NumOfFENs));

  for (auto &Topology : CbmConfiguration.TopologyMapPtr->toValuesList()) {
    if (Topology->Type == CbmType::TTL) {

      std::unique_ptr<EV44Serializer> SerializerPtr =
          std::make_unique<EV44Serializer>(KafkaBufferSize, Topology->Source,
                                           Produce);

      Stats.create("serialize." + Topology->Source + ".produce_called",
                   SerializerPtr->stats().ProduceCalled);
      Stats.create("serialize." + Topology->Source +
                       ".produce_triggered_reftime",
                   SerializerPtr->stats().ProduceRefTimeTriggered);
      Stats.create("serialize." + Topology->Source +
                       ".produce_triggered_max_events",
                   SerializerPtr->stats().ProduceTriggeredMaxEvents);
      Stats.create("serialize." + Topology->Source +
                       ".produce_failed_no_reftime",
                   SerializerPtr->stats().ProduceFailedNoReferenceTime);

      EV44SerializerMapPtr->add(Topology->FEN, Topology->Channel,
                                SerializerPtr);
    } else if (Topology->Type == CbmType::IBM) {

      std::unique_ptr<HistogramSerializer<int32_t>> SerializerPtr =
          std::make_unique<HistogramSerializer<int32_t>>(
              Topology->Source, Topology->maxTofBin, Topology->BinCount, "A",
              Produce);

      Stats.create("serialize." + Topology->Source + ".produce_called",
                   SerializerPtr->stats().ProduceCalled);
      Stats.create("serialize." + Topology->Source +
                       ".tof_before_offset_dropped",
                   SerializerPtr->stats().DataBeforeTimeOffsetDropped);
      Stats.create("serialize." + Topology->Source + ".tof_over_max_dropped",
                   SerializerPtr->stats().DataOverPeriodDropped);
      Stats.create("serialize." + Topology->Source + ".tof_over_max_last_bin",
                   SerializerPtr->stats().DataOverPeriodLastBin);
      Stats.create("serialize." + Topology->Source +
                       ".produce_triggered_reftime",
                   SerializerPtr->stats().ProduceRefTimeTriggered);
      Stats.create("serialize." + Topology->Source +
                       ".produce_failed_no_reftime",
                   SerializerPtr->stats().ProduceFailedNoReferenceTime);

      HistogramSerializerMapPtr->add(Topology->FEN, Topology->Channel,
                                     SerializerPtr);
    }
  }

  // Create instrument
  CbmInstrument cbmInstrument(Counters, CbmConfiguration, *EV44SerializerMapPtr,
                              *HistogramSerializerMapPtr);

  unsigned int DataIndex;
  // Monitor these counters
  TSCTimer ProduceTimer(EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ);
  RuntimeStat RtStat({ITCounters.RxPackets, Counters.CbmCounts,
                      Counters.ProducerStats.produce_bytes_ok});

  while (runThreads) {
    if (InputFifo.pop(DataIndex)) { // There is data in the FIFO - do processing
      auto DataLen = RxRingbuffer.getDataLength(DataIndex);
      if (DataLen == 0) {
        Counters.FifoSeqErrors++;
        continue;
      }

      /// \todo use the Buffer<T> class here and in parser
      auto DataPtr = RxRingbuffer.getDataBuffer(DataIndex);

      int64_t SeqErrOld = Counters.ReadoutStats.ErrorSeqNum;
      auto Res = cbmInstrument.ESSHeaderParser.validate(
          DataPtr, DataLen, cbmInstrument.Conf.Parms.TypeSubType);
      Counters.ReadoutStats = cbmInstrument.ESSHeaderParser.Stats;

      if (SeqErrOld != Counters.ReadoutStats.ErrorSeqNum) {
        XTRACE(DATA, WAR, "SeqNum error at RxPackets %" PRIu64,
               ITCounters.RxPackets);
      }

      if (Res != ESSReadout::Parser::OK) {
        XTRACE(DATA, WAR,
               "Error parsing ESS readout header (RxPackets %" PRIu64 ")",
               ITCounters.RxPackets);
        // hexDump(DataPtr, std::min(64, DataLen));
        Counters.ErrorESSHeaders++;
        continue;
      }

      // We have good header information, now parse readout data
      cbmInstrument.CbmReadoutParser.parse(
          cbmInstrument.ESSHeaderParser.Packet);
      Counters.CbmStats = cbmInstrument.CbmReadoutParser.Stats;
      Counters.TimeStats = cbmInstrument.ESSHeaderParser.Packet.Time.Stats;

      cbmInstrument.processMonitorReadouts();

    } else {
      // There is NO data in the FIFO - increment idle counter and sleep a
      // little
      Counters.ProcessingIdle++;
      usleep(10);
    }

    eventprod.poll(0);

    // Not only flush serializer data but also update runtime stats
    // This not applies for histogram serializer which should be flushed
    // only in case new pulse time is detected
    if (ProduceTimer.timeout()) {
      RuntimeStatusMask = RtStat.getRuntimeStatusMask(
          {ITCounters.RxPackets, Counters.CbmCounts,
           Counters.ProducerStats.produce_bytes_ok});

      for (auto &serializerMap : EV44SerializerMapPtr->toValuesList()) {
        XTRACE(DATA, DEB, "Serializer timed out, producing message now");
        serializerMap->produce();
      }

      Counters.ProduceCauseTimeout++;
      Counters.ProducerStats = eventprod.stats;
    } // ProduceTimer
  }
  XTRACE(INPUT, ALW, "Stopping processing thread.");
  return;
}

} // namespace cbm
