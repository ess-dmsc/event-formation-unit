// Copyright (C) 2021 - 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Freia instrument base plugin
///
//===----------------------------------------------------------------------===//

#include <cinttypes>
#include <common/debug/Trace.h>
#include <common/detector/EFUArgs.h>
#include <common/kafka/KafkaConfig.h>
#include <common/time/TimeString.h>

#include <unistd.h>

#include <common/RuntimeStat.h>
#include <common/memory/SPSCFifo.h>
#include <common/system/Socket.h>
#include <common/time/TSCTimer.h>
#include <common/time/TimeString.h>
#include <common/time/Timer.h>
#include <freia/FreiaBase.h>
#include <freia/FreiaInstrument.h>
#include <stdio.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_WAR

namespace Freia {

const char *classname = "Freia detector with ESS readout";

FreiaBase::FreiaBase(BaseSettings const &settings) : Detector(settings) {

  Stats.setPrefix(EFUSettings.GraphitePrefix, EFUSettings.GraphiteRegion);

  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off

  // Rx and Tx stats
  Stats.create("receive.packets", ITCounters.RxPackets);
  Stats.create("receive.bytes", ITCounters.RxBytes);
  Stats.create("receive.dropped", ITCounters.FifoPushErrors);
  Stats.create("receive.fifo_seq_errors", Counters.FifoSeqErrors);
  Stats.create("transmit.bytes", Counters.TxBytes);
  Stats.create("transmit.monitor_packets", Counters.TxRawReadoutPackets);

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
  Stats.create("readouts.adc_max", Counters.MaxADC);
  Stats.create("readouts.tof_toolarge", Counters.MaxTOFErrors);
  Stats.create("readouts.error_hybrid_mapping", Counters.HybridMappingErrors);
  Stats.create("readouts.error_ring_mapping", Counters.RingMappingErrors);
  Stats.create("readouts.error_fen_mapping", Counters.FENMappingErrors);
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
  // Time stats
  Stats.create("readouts.tof_count", Counters.TimeStats.TofCount);
  Stats.create("readouts.tof_neg", Counters.TimeStats.TofNegative);
  Stats.create("readouts.prevtof_count", Counters.TimeStats.PrevTofCount);
  Stats.create("readouts.prevtof_neg", Counters.TimeStats.PrevTofNegative);


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

  //
  Stats.create("thread.receive_idle", ITCounters.RxIdle);
  Stats.create("thread.processing_idle", Counters.ProcessingIdle);

  // Produce cause call stats
  Stats.create("produce.cause.timeout", Counters.ProduceCauseTimeout);
  Stats.create("produce.cause.pulse_change", Counters.ProduceCausePulseChange);
  Stats.create("produce.cause.max_events_reached", Counters.ProduceCauseMaxEventsReached);

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
  Producer eventprod(EFUSettings.KafkaBroker, EFUSettings.KafkaTopic,
                     KafkaCfg.CfgParms);
  auto Produce = [&eventprod](auto DataBuffer, auto Timestamp) {
    eventprod.produce(DataBuffer, Timestamp);
  };

  Producer MonitorProducer(EFUSettings.KafkaBroker, "freia_debug",
                           KafkaCfg.CfgParms);
  auto ProduceMonitor = [&MonitorProducer](auto DataBuffer, auto Timestamp) {
    MonitorProducer.produce(DataBuffer, Timestamp);
  };

  Serializer = new EV44Serializer(KafkaBufferSize, "freia", Produce);
  MonitorSerializer = new AR51Serializer("freia", ProduceMonitor);

  FreiaInstrument Freia(Counters, EFUSettings, Serializer);

  unsigned int DataIndex;
  TSCTimer ProduceTimer(EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ);
  Timer h5flushtimer;
  // Monitor these counters
  RuntimeStat RtStat({ITCounters.RxPackets, Counters.Events, Counters.TxBytes});

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
      auto Res = Freia.ESSReadoutParser.validate(DataPtr, DataLen,
                                                 ESSReadout::Parser::FREIA);
      Counters.ReadoutStats = Freia.ESSReadoutParser.Stats;

      if (SeqErrOld != Counters.ReadoutStats.ErrorSeqNum) {
        XTRACE(DATA, WAR, "SeqNum error at RxPackets %" PRIu64,
               ITCounters.RxPackets);
      }

      if (Res != ESSReadout::Parser::OK) {
        XTRACE(DATA, WAR,
               "Error parsing ESS readout header (RxPackets %" PRIu64 ")",
               ITCounters.RxPackets);
        Counters.ErrorESSHeaders++;
        continue;
      }

      // We have good header information, now parse readout data
      Res = Freia.VMMParser.parse(Freia.ESSReadoutParser.Packet);
      Counters.TimeStats = Freia.ESSReadoutParser.Packet.Time.Stats;
      Counters.VMMStats = Freia.VMMParser.Stats;

      Freia.processReadouts();

      for (auto &builder : Freia.builders) {
        Freia.generateEvents(builder.Events);
        Counters.MatcherStats.addAndClear(builder.matcher.Stats);
      }
      // done processing data

      // send monitoring data
      if (ITCounters.RxPackets % EFUSettings.MonitorPeriod < EFUSettings.MonitorSamples) {
        XTRACE(PROCESS, DEB, "Serialize and stream monitor data for packet %lu", ITCounters.RxPackets);
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
          {ITCounters.RxPackets, Counters.Events, Counters.TxBytes});

      Serializer->produce();
      Counters.ProduceCauseTimeout++;
      Counters.TxBytes = Serializer->TxBytes;
      Counters.ProduceCausePulseChange = Serializer->ProduceCausePulseChange;
      Counters.ProduceCauseMaxEventsReached = Serializer->ProduceCauseMaxEventsReached;
      Counters.KafkaStats = eventprod.stats;
    }
  }
  XTRACE(INPUT, ALW, "Stopping processing thread.");
  return;
}

} // namespace Freia
