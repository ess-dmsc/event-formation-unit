// Copyright (C) 2022 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief NMX instrument base plugin
///
//===----------------------------------------------------------------------===//

#include <nmx/NMXBase.h>
#include <nmx/NMXInstrument.h>

#include <common/RuntimeStat.h>
#include <common/debug/Trace.h>
#include <common/kafka/KafkaConfig.h>
#include <common/time/Timer.h>
#include <memory>
#include <unistd.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Nmx {

const char *classname = "NMX detector with ESS readout";

NmxBase::NmxBase(BaseSettings const &settings) : Detector(settings) {

  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off


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

  // Clustering stats
  Stats.create("cluster.matched_clusters", Counters.EventsMatchedClusters);
  Stats.create("cluster.no_coincidence", Counters.ClustersNoCoincidence);
  Stats.create("cluster.matched_x_only", Counters.ClustersMatchedXOnly);
  Stats.create("cluster.matched_y_only", Counters.ClustersMatchedYOnly);
  Stats.create("cluster.span.x_too_large", Counters.ClustersTooLargeSpanX);
  Stats.create("cluster.span.y_too_large", Counters.ClustersTooLargeSpanY);
  Stats.create("cluster.span.x_too_small", Counters.ClustersTooSmallSpanX);
  Stats.create("cluster.span.y_too_small", Counters.ClustersTooSmallSpanY);
  Stats.create("cluster.matcherstats.span_too_large", Counters.MatcherStats.SpanTooLarge);
  Stats.create("cluster.matcherstats.discared_span_too_large", Counters.MatcherStats.DiscardedSpanTooLarge);
  Stats.create("cluster.matcherstats.split_span_too_large", Counters.MatcherStats.SplitSpanTooLarge);
  Stats.create("cluster.matcherstats.match_attempt_count", Counters.MatcherStats.MatchAttemptCount);

  // Event stats
  Stats.create("events.count", Counters.Events);
  Stats.create("events.pixel_errors", Counters.PixelErrors);
  Stats.create("events.time_errors", Counters.TimeErrors);

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
    NmxBase::processing_thread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d NMX Rx ringbuffers of size %d",
         EthernetBufferMaxEntries, EthernetBufferSize);
}

void NmxBase::processing_thread() {
  // Event producer
  if (EFUSettings.KafkaTopic == "") {
    XTRACE(INIT, ALW, "EFU is Detector, setting Kafka topic");
    EFUSettings.KafkaTopic = "nmx_detector";
  }

  KafkaConfig KafkaCfg(EFUSettings.KafkaConfigFile);

  Producer EventProducer(EFUSettings.KafkaBroker, EFUSettings.KafkaTopic,
                         KafkaCfg.CfgParms, Stats);
  auto Produce = [&EventProducer](auto DataBuffer, auto Timestamp) {
    EventProducer.produce(DataBuffer, Timestamp);
  };

  Serializer =
      std::make_unique<EV44Serializer>(KafkaBufferSize, "nmx", Produce);

  Stats.create("produce.cause.pulse_change",
               Serializer->stats().ProduceRefTimeTriggered);
  Stats.create("produce.cause.max_events_reached",
               Serializer->stats().ProduceTriggeredMaxEvents);

  NMXInstrument NMX(Counters, EFUSettings, *Serializer, ESSHeaderParser);

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
        ITCounters.FifoSeqErrors++;
        continue;
      }

      /// \todo use the Buffer<T> class here and in parser
      auto DataPtr = RxRingbuffer.getDataBuffer(DataIndex);

      auto Res = ESSHeaderParser.validate(DataPtr, DataLen, DetectorType::NMX);

      if (Res != ESSReadout::Parser::OK) {
        XTRACE(DATA, WAR,
               "Error parsing ESS readout header (RxPackets %" PRIu64 ")",
               getInputCounters().RxPackets);
        // hexDump(DataPtr, std::min(64, DataLen));
        Counters.ErrorESSHeaders++;
        continue;
      }

      // We have good header information, now parse readout data
      Res = NMX.VMMParser.parse(ESSHeaderParser.Packet);
      Counters.VMMStats = NMX.VMMParser.Stats;

      NMX.processReadouts();

      // After each builder has generated events, we add the matcher stats to
      // the global counters, and reset the internal matcher stats to 0
      for (auto &builder : NMX.builders) {
        NMX.generateEvents(builder.Events);
        Counters.MatcherStats.addAndClear(builder.matcher.Stats);
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

} // namespace Nmx
