// Copyright (C) 2021 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Freia instrument base plugin
///
//===----------------------------------------------------------------------===//

#include <cinttypes>
#include <common/detector/EFUArgs.h>
#include <common/kafka/EV42Serializer.h>
#include <common/monitor/HistogramSerializer.h>
#include <common/debug/Hexdump.h>
#include <common/debug/Trace.h>
#include <common/time/TimeString.h>

#include <unistd.h>

#include <common/RuntimeStat.h>
#include <common/system/Socket.h>
#include <common/memory/SPSCFifo.h>
#include <common/time/TimeString.h>
#include <common/time/TSCTimer.h>
#include <common/time/Timer.h>
#include <freia/FreiaBase.h>
#include <freia/FreiaInstrument.h>
#include <stdio.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_WAR

namespace Freia {

const char *classname = "Freia detector with ESS readout";

FreiaBase::FreiaBase(BaseSettings const &settings, struct FreiaSettings &LocalFreiaSettings)
    : Detector("FREIA", settings), FreiaModuleSettings(LocalFreiaSettings) {

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


  #if 0
  // Experiment/Debugging - could be initialised in a loop
  Stats.create("receive.phys_ring0", Counters.RingRx[0]);
  Stats.create("receive.phys_ring1", Counters.RingRx[1]);
  Stats.create("receive.phys_ring2", Counters.RingRx[2]);
  Stats.create("receive.phys_ring3", Counters.RingRx[3]);
  Stats.create("receive.phys_ring4", Counters.RingRx[4]);
  Stats.create("receive.phys_ring5", Counters.RingRx[5]);
  Stats.create("receive.phys_ring6", Counters.RingRx[6]);
  Stats.create("receive.phys_ring7", Counters.RingRx[7]);
  Stats.create("receive.phys_ring8", Counters.RingRx[8]);
  Stats.create("receive.phys_ring9", Counters.RingRx[9]);
  Stats.create("receive.phys_ring10", Counters.RingRx[10]);
  Stats.create("receive.phys_ring11", Counters.RingRx[11]);
  Stats.create("receive.phys_ring12", Counters.RingRx[12]);
  Stats.create("receive.phys_ring13", Counters.RingRx[13]);
  Stats.create("receive.phys_ring14", Counters.RingRx[14]);
  Stats.create("receive.phys_ring15", Counters.RingRx[15]);
  Stats.create("receive.phys_ring16", Counters.RingRx[16]);
  Stats.create("receive.phys_ring17", Counters.RingRx[17]);
  Stats.create("receive.phys_ring18", Counters.RingRx[18]);
  Stats.create("receive.phys_ring19", Counters.RingRx[19]);
  Stats.create("receive.phys_ring20", Counters.RingRx[20]);
  Stats.create("receive.phys_ring21", Counters.RingRx[21]);
  Stats.create("receive.phys_ring22", Counters.RingRx[22]);
  Stats.create("receive.phys_ring23", Counters.RingRx[23]);
  #endif

  // VMM3Parser stats
  Stats.create("monitors.error", Counters.MonitorErrors);
  Stats.create("monitors.count", Counters.MonitorCounts);
  Stats.create("readouts.error_size", Counters.VMMStats.ErrorSize);
  Stats.create("readouts.error_ring", Counters.VMMStats.ErrorRing);
  Stats.create("readouts.ring_mismatch", Counters.RingErrors);
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
  Stats.create("readouts.tof_count", Counters.TimeStats.TofCount);
  Stats.create("readouts.tof_neg", Counters.TimeStats.TofNegative);
  Stats.create("readouts.prevtof_count", Counters.TimeStats.PrevTofCount);
  Stats.create("readouts.prevtof_neg", Counters.TimeStats.PrevTofNegative);
  Stats.create("readouts.tof_toolarge", Counters.TOFErrors);

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
  Stats.create("thread.receive_idle", Counters.RxIdle);
  Stats.create("thread.processing_idle", Counters.ProcessingIdle);


  /// \todo below stats are common to all detectors
  Stats.create("kafka.produce_fails", Counters.KafkaStats.produce_fails);
  Stats.create("kafka.ev_errors", Counters.KafkaStats.ev_errors);
  Stats.create("kafka.ev_others", Counters.KafkaStats.ev_others);
  Stats.create("kafka.dr_errors", Counters.KafkaStats.dr_errors);
  Stats.create("kafka.dr_others", Counters.KafkaStats.dr_noerrors);

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

  std::function<void()> inputFunc = [this]() { FreiaBase::input_thread(); };
  Detector::AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    FreiaBase::processing_thread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d Freia Rx ringbuffers of size %d",
         EthernetBufferMaxEntries, EthernetBufferSize);
}

void FreiaBase::input_thread() {
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


void FreiaBase::processing_thread() {

  // Event producer
  if (EFUSettings.KafkaTopic == "") {
    if (FreiaModuleSettings.IsMonitor) {
      XTRACE(INIT, ALW, "EFU is Monitor, setting Kafka topic");
      EFUSettings.KafkaTopic = "freia_beam_monitor";
    } else {
      XTRACE(INIT, ALW, "EFU is Detector, setting Kafka topic");
      EFUSettings.KafkaTopic = "freia_detector";
    }
  }
  Producer eventprod(EFUSettings.KafkaBroker, EFUSettings.KafkaTopic);
  auto Produce = [&eventprod](auto DataBuffer, auto Timestamp) {
    eventprod.produce(DataBuffer, Timestamp);
  };

  Producer MonitorProducer(EFUSettings.KafkaBroker, "freia_debug");
  auto ProduceMonitor = [&MonitorProducer](auto DataBuffer, auto Timestamp) {
    MonitorProducer.produce(DataBuffer, Timestamp);
  };

  Serializer = new EV42Serializer(KafkaBufferSize, "freia", Produce);
  FreiaInstrument Freia(Counters, /*EFUSettings,*/ FreiaModuleSettings, Serializer);


  HistogramSerializer ADCHistSerializer(Freia.ADCHist.needed_buffer_size(), "Freia");
  ADCHistSerializer.set_callback(ProduceMonitor);

  HistogramSerializer TDCHistSerializer(Freia.TDCHist.needed_buffer_size(), "Freia");
  TDCHistSerializer.set_callback(ProduceMonitor);

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
      auto Res = Freia.ESSReadoutParser.validate(DataPtr, DataLen, ESSReadout::Parser::FREIA);
      Counters.ReadoutStats = Freia.ESSReadoutParser.Stats;

      if (SeqErrOld != Counters.ReadoutStats.ErrorSeqNum) {
        XTRACE(DATA, WAR,"SeqNum error at RxPackets %" PRIu64, Counters.RxPackets);
      }

      if (Res != ESSReadout::Parser::OK) {
        XTRACE(DATA, WAR, "Error parsing ESS readout header (RxPackets %" PRIu64 ")", Counters.RxPackets);
        //hexDump(DataPtr, std::min(64, DataLen));
        Counters.ErrorESSHeaders++;
        continue;
      }

      // We have good header information, now parse readout data
      Res = Freia.VMMParser.parse(Freia.ESSReadoutParser.Packet);
      Counters.TimeStats = Freia.ESSReadoutParser.Packet.Time.Stats;
      Counters.VMMStats = Freia.VMMParser.Stats;


      if (FreiaModuleSettings.IsMonitor) {
        Freia.processMonitorReadouts();
      } else { // process regular events
        Freia.processReadouts();

        for (auto & builder : Freia.builders) {
          Freia.generateEvents(builder.Events);
        }
      }
    } else {
      // There is NO data in the FIFO - increment idle counter and sleep a little
        Counters.ProcessingIdle++;
        usleep(10);
    }

    if (ProduceTimer.timeout()) {

      RuntimeStatusMask =  RtStat.getRuntimeStatusMask(
          {Counters.RxPackets, Counters.Events, Counters.TxBytes});

      Counters.TxBytes += Serializer->produce();
      Counters.KafkaStats = eventprod.stats;

      if (!Freia.ADCHist.isEmpty()) {
        XTRACE(PROCESS, DEB, "Sending ADC histogram for %zu readouts",
           Freia.ADCHist.hit_count());
        ADCHistSerializer.produce(Freia.ADCHist);
        Freia.ADCHist.clear();
      }
      // if (!Freia.TDCHist.isEmpty()) {
      //   XTRACE(PROCESS, DEB, "Sending TDC histogram for %zu readouts",
      //      Freia.TDCHist.hit_count());
      //   TDCHistSerializer.produce(Freia.TDCHist);
      //   Freia.TDCHist.clear();
      // }
    }
  }
  XTRACE(INPUT, ALW, "Stopping processing thread.");
  return;
}

}
