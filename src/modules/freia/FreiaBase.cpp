// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Freia instrument base plugin
///
//===----------------------------------------------------------------------===//

#include <freia/FreiaBase.h>
#include <freia/FreiaInstrument.h>

#include <common/RuntimeStat.h>
#include <common/debug/Trace.h>
#include <common/kafka/KafkaConfig.h>
#include <common/time/Timer.h>
#include <memory>
#include <unistd.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_WAR

namespace Freia {

const char *classname = "Freia detector with ESS readout";

FreiaBase::FreiaBase(BaseSettings const &settings) : Detector(settings) {

  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off

  Stats.create("receive.fifo_seq_errors", Counters.FifoSeqErrors);


  // ESS Readout header stats
  Stats.create("essheader.error_header", Counters.ErrorESSHeaders);

  //
  Stats.create("readouts.adc_max", Counters.MaxADC);
  Stats.create("readouts.tof_toolarge", Counters.MaxTOFErrors);
  Stats.create("readouts.error_hybrid_mapping", Counters.HybridMappingErrors);
  Stats.create("readouts.error_ring_mapping", Counters.RingMappingErrors);
  Stats.create("readouts.error_fen_mapping", Counters.FENMappingErrors);
  Stats.create("readouts.error_invalid_xcoord", Counters.InvalidXCoord);
  Stats.create("readouts.error_invalid_ycoord", Counters.InvalidYCoord);
  // VMM3Parser stats
  Stats.create("readouts.error_size", Counters.VMMStats.ErrorSize);
  Stats.create("readouts.error_fiber", Counters.VMMStats.ErrorFiber);
  Stats.create("readouts.error_fen", Counters.VMMStats.ErrorFEN);
  Stats.create("readouts.error_datalen", Counters.VMMStats.ErrorDataLength);
  Stats.create("readouts.error_timefrac", Counters.VMMStats.ErrorTimeFrac);
  Stats.create("readouts.error_bc", Counters.VMMStats.ErrorBC);
  Stats.create("readouts.error_adc", Counters.VMMStats.ErrorADC);
  Stats.create("readouts.error_vmm", Counters.VMMStats.ErrorVMM);
  Stats.create("readouts.error_channel", Counters.VMMStats.ErrorChannel);
  Stats.create("readouts.count", Counters.VMMStats.Readouts);
  Stats.create("readouts.bccalib", Counters.VMMStats.CalibReadouts);
  Stats.create("readouts.data", Counters.VMMStats.DataReadouts);
  Stats.create("readouts.over_threshold", Counters.VMMStats.OverThreshold);

  // Clustering stats
  Stats.create("cluster.matched_clusters", Counters.EventsMatchedClusters);
  Stats.create("cluster.no_coincidence", Counters.EventsNoCoincidence);
  Stats.create("cluster.wire_only", Counters.EventsMatchedWireOnly);
  Stats.create("cluster.strip_only", Counters.EventsMatchedStripOnly);

  // Event stats
  Stats.create("events.count", Counters.Events);
  Stats.create("events.pixel_errors", Counters.PixelErrors);
  Stats.create("events.time_errors", Counters.TimeErrors);
  Stats.create("events.strip_gaps", Counters.EventsInvalidStripGap);
  Stats.create("events.wire_gaps", Counters.EventsInvalidWireGap);

  // Monitor and calibration stats
  Stats.create("transmit.monitor_packets", Counters.TxRawReadoutPackets);

  //
  Stats.create("thread.processing_idle", Counters.ProcessingIdle);

  // Produce cause call stats
  Stats.create("produce.cause.timeout", Counters.ProduceCauseTimeout);
  /// \todo cleanup and check for unused variables
  //Stats.create("produce.cause.pulse_change", Counters.ProduceCausePulseChange);
  //Stats.create("produce.cause.max_events_reached", Counters.ProduceCauseMaxEventsReached);

  Stats.create("memory.hitvec_storage.alloc_count", HitVectorStorage::Pool->Stats.AllocCount);
  Stats.create("memory.hitvec_storage.alloc_bytes", HitVectorStorage::Pool->Stats.AllocBytes);
  Stats.create("memory.hitvec_storage.dealloc_count", HitVectorStorage::Pool->Stats.DeallocCount);
  Stats.create("memory.hitvec_storage.dealloc_bytes", HitVectorStorage::Pool->Stats.DeallocBytes);
  Stats.create("memory.hitvec_storage.malloc_fallback_count", HitVectorStorage::Pool->Stats.MallocFallbackCount);
  //
  Stats.create("memory.cluster_storage.alloc_count", ClusterPoolStorage::Pool->Stats.AllocCount);
  Stats.create("memory.cluster_storage.alloc_bytes", ClusterPoolStorage::Pool->Stats.AllocBytes);
  Stats.create("memory.cluster_storage.dealloc_count", ClusterPoolStorage::Pool->Stats.DeallocCount);
  Stats.create("memory.cluster_storage.dealloc_bytes", ClusterPoolStorage::Pool->Stats.DeallocBytes);
  Stats.create("memory.cluster_storage.malloc_fallback_count", ClusterPoolStorage::Pool->Stats.MallocFallbackCount);
  // clang-format on

  std::function<void()> inputFunc = [this]() { inputThread(); };
  AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    FreiaBase::processing_thread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d Freia Rx ringbuffers of size %d",
         EthernetBufferMaxEntries, EthernetBufferSize);
}

void FreiaBase::processing_thread() {

  // Event producer
  assert(EFUSettings.KafkaTopic != "");

  KafkaConfig KafkaCfg(EFUSettings.KafkaConfigFile);
  Producer EventProducer(EFUSettings.KafkaBroker, EFUSettings.KafkaTopic,
                         KafkaCfg.CfgParms, &Stats);
  auto Produce = [&EventProducer](const auto &DataBuffer,
                                  const auto &Timestamp) {
    EventProducer.produce(DataBuffer, Timestamp);
  };

  Producer MonitorProducer(EFUSettings.KafkaBroker, EFUSettings.KafkaDebugTopic,
                           KafkaCfg.CfgParms);
  auto ProduceMonitor = [&MonitorProducer](auto &DataBuffer, auto Timestamp) {
    MonitorProducer.produce(DataBuffer, Timestamp);
  };

  Serializer = std::make_unique<EV44Serializer>(KafkaBufferSize,
                                                FlatBufferSource, Produce);

  Stats.create("produce.cause.pulse_change",
               Serializer->stats().ProduceRefTimeTriggered);
  Stats.create("produce.cause.max_events_reached",
               Serializer->stats().ProduceTriggeredMaxEvents);

  MonitorSerializer = std::make_unique<AR51Serializer>("freia", ProduceMonitor);

  FreiaInstrument Freia(Counters, EFUSettings, *Serializer, ESSHeaderParser);

  unsigned int DataIndex;

  // Time out after one second
  Timer ProduceTimer(EFUSettings.UpdateIntervalSec * 1'000'000'000);

  // Monitor these counters
  RuntimeStat RtStat({getInputCounters().RxPackets, Counters.Events,
                      EventProducer.getStats().MsgStatusPersisted});

  // Set the datatype
  uint8_t DataType{DetectorType::FREIA};

  if (EFUSettings.DetectorName == "freia") {
    DataType = DetectorType::FREIA;
  }

  else if (EFUSettings.DetectorName == "estia") {
    DataType = DetectorType::ESTIA;
  }

  else if (EFUSettings.DetectorName == "tblmb") {
    DataType = DetectorType::TBLMB;
  }

  while (runThreads) {
    if (InputFifo.pop(DataIndex)) { // There is data in the FIFO - do processing
      auto DataLen = RxRingbuffer.getDataLength(DataIndex);
      if (DataLen == 0) {
        Counters.FifoSeqErrors++;
        continue;
      }

      /// \todo use the Buffer<T> class here and in parser
      auto DataPtr = RxRingbuffer.getDataBuffer(DataIndex);

      auto Res = ESSHeaderParser.validate(DataPtr, DataLen, DataType);

      if (Res != ESSReadout::Parser::OK) {
        XTRACE(DATA, WAR,
               "Error parsing ESS readout header (RxPackets %" PRIu64 ")",
               getInputCounters().RxPackets);
        Counters.ErrorESSHeaders++;
        continue;
      }

      // We have good header information, now parse readout data
      Res = Freia.VMMParser.parse(ESSHeaderParser.Packet);
      Counters.VMMStats = Freia.VMMParser.Stats;

      Freia.processReadouts();

      for (auto &builder : Freia.builders) {
        Freia.generateEvents(builder.Events);
        Counters.MatcherStats.addAndClear(builder.matcher.Stats);
      }
      // done processing data

      // send monitoring data
      if (getInputCounters().RxPackets % EFUSettings.MonitorPeriod <
          EFUSettings.MonitorSamples) {
        XTRACE(PROCESS, DEB, "Serialize and stream monitor data for packet %lu",
               getInputCounters().RxPackets);
        MonitorSerializer->serialize((uint8_t *)DataPtr, DataLen);
        MonitorSerializer->produce();
        Counters.TxRawReadoutPackets++;
      }
    } else {
      // There is NO data in the FIFO - increment idle counter and sleep a
      // little
      Counters.ProcessingIdle++;
      usleep(10);
    }

    if (ProduceTimer.timeout()) {

      RuntimeStatusMask = RtStat.getRuntimeStatusMask(
          {getInputCounters().RxPackets, Counters.Events,
           EventProducer.getStats().MsgStatusPersisted});

      Serializer->produce();
      Counters.ProduceCauseTimeout++;
    }
  }
  XTRACE(INPUT, ALW, "Stopping processing thread.");
  return;
}

} // namespace Freia
