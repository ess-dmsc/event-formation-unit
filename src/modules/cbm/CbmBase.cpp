// Copyright (C) 2022 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CBM instrument base class which define the detector and required data
///        processing
///
//===----------------------------------------------------------------------===//

#include <modules/cbm/CbmBase.h>
#include <modules/cbm/CbmInstrument.h>

#include <common/RuntimeStat.h>
#include <common/kafka/EV44Serializer.h>
#include <common/kafka/KafkaConfig.h>
#include <common/memory/HashMap2D.h>

#include <memory>
#include <unistd.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace cbm {

using namespace fbserializer;

CbmBase::CbmBase(BaseSettings const &settings)
    : Detector(settings), CbmConfiguration(EFUSettings.ConfigFile) {
  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off
  // Readout parsing errors - readout dropped
  Stats.create("parser.readout.count", Counters.CbmStats.Readouts);
  Stats.create("parser.readout.header.count", Counters.DataHeaders);
  Stats.create("parser.readout.errors.size", Counters.CbmStats.ErrorSize);
  Stats.create("parser.readout.errors.fiber", Counters.CbmStats.ErrorFiber);
  Stats.create("parser.readout.errors.fen", Counters.CbmStats.ErrorFEN);
  Stats.create("parser.readout.errors.type", Counters.CbmStats.ErrorType);
  Stats.create("parser.readout.errors.adc", Counters.CbmStats.ErrorADC);
  Stats.create("parser.readout.errors.datalen", Counters.CbmStats.ErrorDataLength);
  Stats.create("parser.readout.errors.timefrac", Counters.CbmStats.ErrorTimeFrac);
  Stats.create("parser.readout.errors.no_data", Counters.CbmStats.NoData);

  // Readout processing stats
  Stats.create("readouts.processed", Counters.CbmCounts);
  Stats.create("readouts.type.event0d_proccessed", Counters.Event0DReadoutsProcessed);
  Stats.create("readouts.type.event2d_proccessed", Counters.Event2DReadoutsProcessed);
  Stats.create("readouts.type.ibm_processed", Counters.IBMReadoutsProcessed);

  // Published events and Ar51
  Stats.create("events.ibm", Counters.IBMEvents);
  Stats.create("events.event0d", Counters.Event0DEvents);
  Stats.create("events.event2d", Counters.Event2DEvents);
  Stats.create("events.ibm_npos_sum", Counters.NPOSCount);

  // Readout processing errors - readout dropped
  Stats.create("readouts.errors.ring_mismatch", Counters.RingCfgError);
  Stats.create("readouts.errors.no_serializer", Counters.NoSerializerCfgError);
  Stats.create("readouts.errors.type_not_configured", Counters.TypeNotConfigured);
  Stats.create("readouts.errors.time", Counters.TimeError);

  //
  Stats.create("thread.processing_idle", Counters.ProcessingIdle);

  // Produce cause call stats
  Stats.create("produce.cause.timeout", Counters.ProduceCauseTimeout);

  // clang-format on
  std::function<void()> inputFunc = [this]() { inputThread(); };
  AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    CbmBase::processingThread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d CBM Rx ringbuffers of size %d",
         EthernetBufferMaxEntries, EthernetBufferSize);
}

void CbmBase::processingThread() {
  /// \todo: properly the quite and stop application and log the fault.
  if (EFUSettings.KafkaTopic == "") {
    XTRACE(INPUT, ALW, "Missing topic - mandatory for beam monitor");
    stopThreads();
  }
  XTRACE(INPUT, ALW, "Kafka topic %s", EFUSettings.KafkaTopic.c_str());

  // ---------------------------------------------------------------------------
  // Event producer
  KafkaConfig KafkaCfg(EFUSettings.KafkaConfigFile);
  Producer EventProducer(EFUSettings.KafkaBroker, EFUSettings.KafkaTopic,
                         KafkaCfg.CfgParms, Stats);

  auto Produce = [&EventProducer](const auto &DataBuffer,
                                  const auto &Timestamp) {
    EventProducer.produce(DataBuffer, Timestamp);
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
    if ((Topology->Type == CbmType::EVENT_0D) || (Topology->Type == CbmType::EVENT_2D)) {

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

      essmath::VectorAggregationFunc<int32_t> AggFunc =
          essmath::SUM_AGG_FUNC<int32_t>;
      if (Topology->AggregationMode == AggregationType::AVG) {
        AggFunc = essmath::AVERAGE_AGG_FUNC<int32_t>;
      }

      std::unique_ptr<HistogramSerializer<int32_t>> SerializerPtr =
          std::make_unique<HistogramSerializer<int32_t>>(
              Topology->Source, Topology->maxTofBin, Topology->BinCount, "A",
              Topology->AggregatedFrames, Produce, 0, AggFunc);


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
                              *HistogramSerializerMapPtr, ESSHeaderParser);

  // Monitor these counters and time out after one second
  Timer ProduceTimer(EFUSettings.UpdateIntervalSec * 1'000'000'000);
  RuntimeStat RtStat({getInputCounters().RxPackets, Counters.CbmCounts,
                      EventProducer.getStats().MsgStatusPersisted});

  unsigned int DataIndex;
  while (runThreads) {
    if (InputFifo.pop(DataIndex)) { // There is data in the FIFO - do processing
      auto DataLen = RxRingbuffer.getDataLength(DataIndex);
      if (DataLen == 0) {
        ITCounters.FifoSeqErrors++;
        continue;
      }

      /// \todo use the Buffer<T> class here and in parser
      auto DataPtr = RxRingbuffer.getDataBuffer(DataIndex);

      auto Res = ESSHeaderParser.validate(DataPtr, DataLen,
                                          CbmConfiguration.Parms.TypeSubType);

      if (Res != ESSReadout::Parser::OK) {
        XTRACE(DATA, WAR,
               "Error parsing ESS readout header (RxPackets %" PRIu64 ")",
               getInputCounters().RxPackets);
        // hexDump(DataPtr, std::min(64, DataLen));
        continue;
      }

      // We have good header information, now parse readout data
      cbmInstrument.CbmReadoutParser.parse(ESSHeaderParser.Packet);
      Counters.CbmStats = cbmInstrument.CbmReadoutParser.Stats;

      cbmInstrument.processMonitorReadouts();

    } else {
      // There is NO data in the FIFO - increment idle counter and sleep a
      // little
      Counters.ProcessingIdle++;
      usleep(10);
    }

    EventProducer.poll(0);

    // Not only flush serializer data but also update runtime stats
    // This not applies for histogram serializer which should be flushed
    // only in case new pulse time is detected
    if (ProduceTimer.timeout()) {
      RuntimeStatusMask = RtStat.getRuntimeStatusMask(
          {getInputCounters().RxPackets, Counters.CbmCounts,
           EventProducer.getStats().MsgStatusPersisted});

      for (auto &serializerMap : EV44SerializerMapPtr->toValuesList()) {
        XTRACE(DATA, DEB, "Serializer timed out, producing message now");
        serializerMap->produce();
      }

      Counters.ProduceCauseTimeout++;
    } // ProduceTimer
  }
  XTRACE(INPUT, ALW, "Stopping processing thread.");
  return;
}

} // namespace cbm
