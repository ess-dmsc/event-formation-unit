// Copyright (C) 2022 - 2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CBM instrument base class which define the detector and required data
///        processing
//===----------------------------------------------------------------------===//

#include "common/kafka/EV44Serializer.h"
#include "common/memory/HashMap2D.h"
#include <common/RuntimeStat.h>
#include <common/kafka/KafkaConfig.h>
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

  //
  Stats.create("monitors.count", Counters.MonitorCounts);
  Stats.create("monitors.reduced", Counters.MonitorIgnored);
  Stats.create("readouts.adc_max", Counters.MaxADC);
  Stats.create("readouts.tof_toolarge", Counters.TOFErrors);
  Stats.create("readouts.ring_mismatch", Counters.RingCfgErrors);

  Stats.create("readouts.error_size", Counters.CbmStats.ErrorSize);
  Stats.create("readouts.error_fiber", Counters.CbmStats.ErrorFiber);
  Stats.create("readouts.error_fen", Counters.CbmStats.ErrorFEN);
  Stats.create("readouts.error_type", Counters.CbmStats.ErrorType);
  Stats.create("readouts.error_adc", Counters.CbmStats.ErrorADC);
  Stats.create("readouts.error_datalen", Counters.CbmStats.ErrorDataLength);
  Stats.create("readouts.error_timefrac", Counters.CbmStats.ErrorTimeFrac);
  Stats.create("readouts.count", Counters.CbmStats.Readouts);
  Stats.create("readouts.empty", Counters.CbmStats.NoData);


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

  // Serializer stats
  Stats.create("serializer.no_serializer_configured", Counters.NoSerializerCfgError);

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
    CbmBase::processing_thread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d CBM Rx ringbuffers of size %d",
         EthernetBufferMaxEntries, EthernetBufferSize);
}

void CbmBase::processing_thread() {

  if (EFUSettings.KafkaTopic == "") {
    XTRACE(INPUT, ALW, "Missing topic - mandatory for beam monitor");
    stopThreads();
  }
  XTRACE(INPUT, ALW, "Kafka topic %s", EFUSettings.KafkaTopic.c_str());

  // Event producer
  KafkaConfig KafkaCfg(EFUSettings.KafkaConfigFile);
  Producer eventprod(EFUSettings.KafkaBroker, EFUSettings.KafkaTopic,
                     KafkaCfg.CfgParms);

  auto Produce = [&eventprod](auto DataBuffer, auto Timestamp) {
    eventprod.produce(DataBuffer, Timestamp);
  };

  // Process instrument config file
  XTRACE(INIT, ALW, "Loading configuration file %s",
         EFUSettings.ConfigFile.c_str());

  CbmConfiguration.loadAndApply();

  // Create serializers
  EV44SerializerMapPtr.reset(
      new HashMap2D<EV44Serializer>(CbmConfiguration.Parms.MaxFENId));
  HistogramSerializerMapPtr.reset(new HashMap2D<HistogramSerializer<int32_t>>(
      CbmConfiguration.Parms.MaxFENId));

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
              Topology->Source, Topology->maxTofBin, Topology->BinCount,
              "serializer", "A", "ns", Produce);

      Stats.create("serialize." + Topology->Source + ".produce_called",
                   SerializerPtr->stats().ProduceCalled);
      Stats.create("serialize." + Topology->Source + "tof_over_max_drop",
                   SerializerPtr->stats().DataOverPeriodDropped);
      Stats.create("serialize." + Topology->Source + "tof_over_max_last_bin",
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
  RuntimeStat RtStat({ITCounters.RxPackets, Counters.MonitorCounts,
                      Counters.KafkaStats.produce_bytes_ok});

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
      auto Res = cbmInstrument.ESSReadoutParser.validate(
          DataPtr, DataLen, cbmInstrument.Conf.Parms.TypeSubType);
      Counters.ReadoutStats = cbmInstrument.ESSReadoutParser.Stats;

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
      cbmInstrument.CbmParser.parse(cbmInstrument.ESSReadoutParser.Packet);
      Counters.CbmStats = cbmInstrument.CbmParser.Stats;
      Counters.TimeStats = cbmInstrument.ESSReadoutParser.Packet.Time.Stats;

      cbmInstrument.processMonitorReadouts();

    } else {
      // There is NO data in the FIFO - increment idle counter and sleep a
      // little
      Counters.ProcessingIdle++;
      usleep(10);
    }

    // Not only flush serializer data but also update runtime stats
    if (ProduceTimer.timeout()) {
      RuntimeStatusMask = RtStat.getRuntimeStatusMask(
          {ITCounters.RxPackets, Counters.MonitorCounts,
           Counters.KafkaStats.produce_bytes_ok});

      for (auto &serializerMap : EV44SerializerMapPtr->toValuesList()) {
        XTRACE(DATA, DEB, "Serializer timed out, producing message now");
        serializerMap->produce();
      }

      for (auto &serializerMap : HistogramSerializerMapPtr->toValuesList()) {
        XTRACE(DATA, DEB, "Serializer timed out, producing message now");
        serializerMap->produce();
      }
      Counters.ProduceCauseTimeout++;
      Counters.KafkaStats = eventprod.stats;
    } // ProduceTimer
  }
  XTRACE(INPUT, ALW, "Stopping processing thread.");
  return;
}

} // namespace cbm
