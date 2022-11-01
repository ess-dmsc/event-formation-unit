// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CSPEC instrument base plugin
///
//===----------------------------------------------------------------------===//

#include <common/RuntimeStat.h>
#include <common/debug/Hexdump.h>
#include <common/debug/Trace.h>
#include <common/detector/EFUArgs.h>
#include <common/kafka/EV42Serializer.h>
#include <common/kafka/KafkaConfig.h>
#include <common/memory/SPSCFifo.h>
#include <common/monitor/HistogramSerializer.h>
#include <common/system/Socket.h>
#include <common/time/TSCTimer.h>
#include <common/time/TimeString.h>
#include <common/time/Timer.h>
#include <cspec/CSPECBase.h>
#include <cspec/CSPECInstrument.h>
#include <stdio.h>
#include <unistd.h>

#include <cinttypes>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_WAR

namespace Cspec {

const char *classname = "CSPEC detector with ESS readout";

CspecBase::CspecBase(BaseSettings const &settings)
    : Detector("CSPEC", settings) {
  Stats.setPrefix(EFUSettings.GraphitePrefix, EFUSettings.GraphiteRegion);

  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off

  // Rx and Tx stats
  Stats.create("receive.packets", Counters.RxPackets);
  Stats.create("receive.bytes", Counters.RxBytes);
  Stats.create("receive.dropped", Counters.FifoPushErrors);
  Stats.create("receive.fifo_seq_errors", Counters.FifoSeqErrors);
  Stats.create("transmit.bytes", Counters.TxBytes);

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

  //
  Stats.create("readouts.adc_max", Counters.MaxADC);
  Stats.create("readouts.error_mapping", Counters.HybridMappingErrors);
  // VMM3Parser stats
  Stats.create("readouts.count", Counters.VMMStats.Readouts);
  Stats.create("readouts.error_ring", Counters.VMMStats.ErrorRing);
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
  Stats.create("readouts.tof_count", Counters.TimeStats.TofCount);
  Stats.create("readouts.tof_neg", Counters.TimeStats.TofNegative);
  Stats.create("readouts.prevtof_count", Counters.TimeStats.PrevTofCount);
  Stats.create("readouts.prevtof_neg", Counters.TimeStats.PrevTofNegative);
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

  //
  Stats.create("thread.receive_idle", Counters.RxIdle);
  Stats.create("thread.processing_idle", Counters.ProcessingIdle);


  /// \todo below stats are common to all detectors
  Stats.create("kafka.produce_fails", Counters.KafkaStats.produce_fails);
  Stats.create("kafka.ev_errors", Counters.KafkaStats.ev_errors);
  Stats.create("kafka.ev_others", Counters.KafkaStats.ev_others);
  Stats.create("kafka.dr_errors", Counters.KafkaStats.dr_errors);
  Stats.create("kafka.dr_others", Counters.KafkaStats.dr_noerrors);

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

  std::function<void()> inputFunc = [this]() { CspecBase::input_thread(); };
  Detector::AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    CspecBase::processing_thread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d CSPEC Rx ringbuffers of size %d",
         EthernetBufferMaxEntries, EthernetBufferSize);
}

void CspecBase::input_thread() {
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(),
                         EFUSettings.DetectorPort);
  UDPReceiver receiver(local);
  receiver.setBufferSizes(EFUSettings.TxSocketBufferSize,
                          EFUSettings.RxSocketBufferSize);
  receiver.checkRxBufferSizes(EFUSettings.RxSocketBufferSize);
  receiver.printBufferSizes();
  receiver.setRecvTimeout(0, 100000); /// secs, usecs 1/10s

  while (runThreads) {
    int readSize;
    unsigned int rxBufferIndex = RxRingbuffer.getDataIndex();

    // this is the processing step
    RxRingbuffer.setDataLength(rxBufferIndex, 0);
    if ((readSize = receiver.receive(RxRingbuffer.getDataBuffer(rxBufferIndex),
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

void CspecBase::processing_thread() {
  // Event producer
  if (EFUSettings.KafkaTopic == "") {
    XTRACE(INIT, ALW, "EFU is Detector, setting Kafka topic");
    EFUSettings.KafkaTopic = "cspec_detector";
  }

  KafkaConfig KafkaCfg(EFUSettings.KafkaConfigFile);
  Producer eventprod(EFUSettings.KafkaBroker, EFUSettings.KafkaTopic,
    KafkaCfg.CfgParms);
  auto Produce = [&eventprod](auto DataBuffer, auto Timestamp) {
    eventprod.produce(DataBuffer, Timestamp);
  };

  Producer MonitorProducer(EFUSettings.KafkaBroker, "cspec_debug",
    KafkaCfg.CfgParms);
  auto ProduceMonitor = [&MonitorProducer](auto DataBuffer, auto Timestamp) {
    MonitorProducer.produce(DataBuffer, Timestamp);
  };

  Serializer = new EV42Serializer(KafkaBufferSize, "cspec", Produce);
  CSPECInstrument CSPEC(Counters, EFUSettings, Serializer);

  HistogramSerializer ADCHistSerializer(CSPEC.ADCHist.needed_buffer_size(),
                                        "CSPEC");
  ADCHistSerializer.set_callback(ProduceMonitor);

  unsigned int DataIndex;
  TSCTimer ProduceTimer(EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ);
  Timer h5flushtimer;
  // Monitor these counters
  RuntimeStat RtStat({Counters.RxPackets, Counters.Events, Counters.TxBytes});

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
      auto Res = CSPEC.ESSReadoutParser.validate(DataPtr, DataLen,
                                                 ESSReadout::Parser::CSPEC);
      Counters.ReadoutStats = CSPEC.ESSReadoutParser.Stats;

      if (SeqErrOld != Counters.ReadoutStats.ErrorSeqNum) {
        XTRACE(DATA, WAR, "SeqNum error at RxPackets %" PRIu64,
               Counters.RxPackets);
      }

      if (Res != ESSReadout::Parser::OK) {
        XTRACE(DATA, WAR,
               "Error parsing ESS readout header (RxPackets %" PRIu64 ")",
               Counters.RxPackets);
        // hexDump(DataPtr, std::min(64, DataLen));
        Counters.ErrorESSHeaders++;
        continue;
      }

      // We have good header information, now parse readout data
      Res = CSPEC.VMMParser.parse(CSPEC.ESSReadoutParser.Packet);
      Counters.TimeStats = CSPEC.ESSReadoutParser.Packet.Time.Stats;
      Counters.VMMStats = CSPEC.VMMParser.Stats;

      CSPEC.processReadouts();

      for (auto &builder : CSPEC.builders) {
        CSPEC.generateEvents(builder.Events);
      }

    } else {
      // There is NO data in the FIFO - increment idle counter and sleep a
      // little
      Counters.ProcessingIdle++;
      usleep(10);
    }

    if (ProduceTimer.timeout()) {
      RuntimeStatusMask = RtStat.getRuntimeStatusMask(
          {Counters.RxPackets, Counters.Events, Counters.TxBytes});

      Counters.TxBytes += Serializer->produce();
      Counters.KafkaStats = eventprod.stats;

      if (!CSPEC.ADCHist.isEmpty()) {
        XTRACE(PROCESS, DEB, "Sending ADC histogram for %zu readouts",
               CSPEC.ADCHist.hit_count());
        ADCHistSerializer.produce(CSPEC.ADCHist);
        CSPEC.ADCHist.clear();
      }
      // if (!CSPEC.TDCHist.isEmpty()) {
      //   XTRACE(PROCESS, DEB, "Sending TDC histogram for %zu readouts",
      //      CSPEC.TDCHist.hit_count());
      //   TDCHistSerializer.produce(CSPEC.TDCHist);
      //   CSPEC.TDCHist.clear();
      // }
    }
  }
  XTRACE(INPUT, ALW, "Stopping processing thread.");
  return;
}

} // namespace Cspec
