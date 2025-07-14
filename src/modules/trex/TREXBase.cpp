// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief TREX instrument base plugin
///
//===----------------------------------------------------------------------===//

#include <memory>
#include <trex/TREXBase.h>
#include <trex/TREXInstrument.h>

#include <common/RuntimeStat.h>
#include <common/debug/Trace.h>
#include <common/detector/EFUArgs.h>
#include <common/kafka/EV44Serializer.h>
#include <common/kafka/KafkaConfig.h>
#include <common/memory/SPSCFifo.h>
#include <common/monitor/HistogramSerializer.h>
#include <common/system/SocketImpl.h>
#include <common/time/TimeString.h>
#include <common/time/Timer.h>

#include <cinttypes>
#include <stdio.h>
#include <unistd.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_WAR

namespace Trex {

const char *classname = "TREX detector with ESS readout";

TrexBase::TrexBase(BaseSettings const &settings) : Detector(settings) {
  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off

  Stats.create("receive.fifo_seq_errors", Counters.FifoSeqErrors);

  // ESS Readout header stats
  Stats.create("essheader.error_header", Counters.ErrorESSHeaders);

  //
  Stats.create("readouts.adc_max", Counters.MaxADC);
  Stats.create("readouts.error_mapping", Counters.HybridMappingErrors);
  // VMM3Parser stats
  Stats.create("readouts.count", Counters.VMMStats.Readouts);
  Stats.create("readouts.error_fiber", Counters.VMMStats.ErrorFiber);
  Stats.create("readouts.error_fen", Counters.VMMStats.ErrorFEN);
  Stats.create("readouts.error_datalen", Counters.VMMStats.ErrorDataLength);
  Stats.create("readouts.error_timefrac", Counters.VMMStats.ErrorTimeFrac);
  Stats.create("readouts.error_bc", Counters.VMMStats.ErrorBC);
  Stats.create("readouts.error_adc", Counters.VMMStats.ErrorADC);
  Stats.create("readouts.error_vmm", Counters.VMMStats.ErrorVMM);
  Stats.create("readouts.error_channel", Counters.VMMStats.ErrorChannel);
  Stats.create("readouts.error_size", Counters.VMMStats.ErrorSize);
  Stats.create("readouts.data", Counters.VMMStats.DataReadouts);
  Stats.create("readouts.bccalib", Counters.VMMStats.CalibReadouts);
  Stats.create("readouts.over_threshold", Counters.VMMStats.OverThreshold);


  // Time stats
  Stats.create("readouts.tof_toolarge", Counters.TOFErrors); //move this to events.tof_toolarge


  // Clustering stats
  Stats.create("cluster.matched_clusters", Counters.EventsMatchedClusters);
  Stats.create("cluster.no_coincidence", Counters.ClustersNoCoincidence);
  Stats.create("cluster.matched_wire_only", Counters.ClustersMatchedWireOnly);
  Stats.create("cluster.matched_grid_only", Counters.ClustersMatchedGridOnly);
  Stats.create("cluster.too_large_grid_span", Counters.ClustersTooLargeGridSpan);

  // Event stats
  Stats.create("events.count", Counters.Events);
  Stats.create("events.pixel_errors", Counters.PixelErrors);
  Stats.create("events.time_errors", Counters.TimeErrors);

  // Monitor and calibration stats
  Stats.create("transmit.monitor_packets", Counters.TxRawReadoutPackets);

  //
  Stats.create("thread.processing_idle", Counters.ProcessingIdle);

  // Produce cause call stats
  Stats.create("produce.cause.timeout", Counters.ProduceCauseTimeout);

  // Stats.create("memory.hitvec_storage.alloc_count", HitVectorStorage::Pool->Stats.AllocCount);
  // Stats.create("memory.hitvec_storage.alloc_bytes", HitVectorStorage::Pool->Stats.AllocBytes);
  // Stats.create("memory.hitvec_storage.dealloc_count", HitVectorStorage::Pool->Stats.DeallocCount);
  // Stats.create("memory.hitvec_storage.dealloc_bytes", HitVectorStorage::Pool->Stats.DeallocBytes);
  // Stats.create("memory.hitvec_storage.malloc_fallback_count", HitVectorStorage::Pool->Stats.MallocFallbackCount);
  //
  // Stats.create("memory.cluster_storage.alloc_count", ClusterPoolStorage::Pool->Stats.AllocCount);
  // Stats.create("memory.cluster_storage.alloc_bytes", ClusterPoolStorage::Pool->Stats.AllocBytes);
  // Stats.create("memory.cluster_storage.dealloc_count", ClusterPoolStorage::Pool->Stats.DeallocCount);
  // Stats.create("memory.cluster_storage.dealloc_bytes", ClusterPoolStorage::Pool->Stats.DeallocBytes);
  // Stats.create("memory.cluster_storage.malloc_fallback_count", ClusterPoolStorage::Pool->Stats.MallocFallbackCount);

  // clang-format on
  std::function<void()> inputFunc = [this]() { inputThread(); };
  AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    TrexBase::processing_thread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d TREX Rx ringbuffers of size %d",
         EthernetBufferMaxEntries, EthernetBufferSize);
}

void TrexBase::processing_thread() {
  // Event producer
  if (EFUSettings.KafkaTopic == "") {
    XTRACE(INIT, ALW, "EFU is Detector, setting Kafka topic");
    EFUSettings.KafkaTopic = "trex_detector";
  }

  KafkaConfig KafkaCfg(EFUSettings.KafkaConfigFile);
  Producer EventProducer(EFUSettings.KafkaBroker, EFUSettings.KafkaTopic,
                         KafkaCfg.CfgParms, &Stats);
  auto Produce = [&EventProducer](const auto &DataBuffer,
                                  const auto &Timestamp) {
    EventProducer.produce(DataBuffer, Timestamp);
  };

  Producer MonitorProducer(EFUSettings.KafkaBroker, "trex_debug",
                           KafkaCfg.CfgParms);
  auto ProduceMonitor = [&MonitorProducer](auto DataBuffer, auto Timestamp) {
    MonitorProducer.produce(DataBuffer, Timestamp);
  };

  Serializer =
      std::make_unique<EV44Serializer>(KafkaBufferSize, "trex", Produce);

  Stats.create("produce.cause.pulse_change",
               Serializer->stats().ProduceRefTimeTriggered);
  Stats.create("produce.cause.max_events_reached",
               Serializer->stats().ProduceTriggeredMaxEvents);

  TREXInstrument TREX(Counters, EFUSettings, *Serializer, ESSHeaderParser);

  HistogramSerializer ADCHistSerializer(TREX.ADCHist.needed_buffer_size(),
                                        "TREX");
  ADCHistSerializer.set_callback(ProduceMonitor);

  // Time out after one second
  Timer ProduceTimer(EFUSettings.UpdateIntervalSec * 1'000'000'000);

  // Monitor these counters
  RuntimeStat RtStat({getInputCounters().RxPackets, Counters.Events,
                      EventProducer.getStats().MsgStatusPersisted});

  unsigned int DataIndex;
  while (runThreads) {
    if (InputFifo.pop(DataIndex)) { // There is data in the FIFO - do processing
      auto DataLen = RxRingbuffer.getDataLength(DataIndex);
      if (DataLen == 0) {
        Counters.FifoSeqErrors++;
        continue;
      }

      /// \todo use the Buffer<T> class here and in parser
      auto DataPtr = RxRingbuffer.getDataBuffer(DataIndex);

      auto Res = ESSHeaderParser.validate(DataPtr, DataLen, DetectorType::TREX);

      if (Res != ESSReadout::Parser::OK) {
        XTRACE(DATA, WAR,
               "Error parsing ESS readout header (RxPackets %" PRIu64 ")",
               getInputCounters().RxPackets);
        // hexDump(DataPtr, std::min(64, DataLen));
        Counters.ErrorESSHeaders++;
        continue;
      }

      // We have good header information, now parse readout data
      Res = TREX.VMMParser.parse(ESSHeaderParser.Packet);
      Counters.VMMStats = TREX.VMMParser.Stats;

      TREX.processReadouts();

      for (auto &builder : TREX.builders) {
        TREX.generateEvents(builder.Events);
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

      if (!TREX.ADCHist.isEmpty()) {
        XTRACE(PROCESS, DEB, "Sending ADC histogram for %zu readouts",
               TREX.ADCHist.hitCount());
        ADCHistSerializer.produce(TREX.ADCHist);
        TREX.ADCHist.clear();
      }
      // if (!TREX.TDCHist.isEmpty()) {
      //   XTRACE(PROCESS, DEB, "Sending TDC histogram for %zu readouts",
      //      TREX.TDCHist.hitCount());
      //   TDCHistSerializer.produce(TREX.TDCHist);
      //   TREX.TDCHist.clear();
      // }
    }
  }
  XTRACE(INPUT, ALW, "Stopping processing thread.");
  return;
}

} // namespace Trex
