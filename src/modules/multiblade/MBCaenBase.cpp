// Copyright (C) 2018-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Multi-Blade prototype detector base plugin interface definition
///
//===----------------------------------------------------------------------===//


#include <cinttypes>
#include <common/detector/EFUArgs.h>
#include <common/kafka/EV42Serializer.h>
#include <common/kafka/Producer.h>
#include <common/debug/Trace.h>
#include <common/time/TimeString.h>
#include <common/TestImageUdder.h>

#include <unistd.h>

#include <common/RuntimeStat.h>
#include <common/system/Socket.h>
#include <common/memory/SPSCFifo.h>
#include <common/time/TimeString.h>
#include <common/time/TSCTimer.h>
#include <common/time/Timer.h>
#include <multiblade/MBCaenBase.h>
#include <multiblade/MBCaenInstrument.h>
#include <unistd.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Multiblade {

const char *classname = "Multiblade detector with CAEN readout";

CAENBase::CAENBase(BaseSettings const &settings, struct CAENSettings &LocalMBCAENSettings)
    : Detector("MBCAEN", settings), MBCAENSettings(LocalMBCAENSettings) {

  Stats.setPrefix(EFUSettings.GraphitePrefix, EFUSettings.GraphiteRegion);

  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off
  Stats.create("receive.packets", Counters.RxPackets);
  Stats.create("receive.bytes", Counters.RxBytes);
  Stats.create("receive.idle", Counters.RxIdle);
  Stats.create("receive.dropped", Counters.FifoPushErrors);
  Stats.create("receive.fifo_seq_errors", Counters.FifoSeqErrors);

  Stats.create("receive.packet_bad_header", Counters.PacketBadDigitizer);
  Stats.create("readouts.count", Counters.ReadoutsCount);
  Stats.create("readouts.count_valid", Counters.ReadoutsGood);
  Stats.create("readouts.invalid_ch", Counters.ReadoutsInvalidChannel);
  Stats.create("readouts.invalid_adc", Counters.ReadoutsInvalidAdc);
  Stats.create("readouts.invalid_plane", Counters.ReadoutsInvalidPlane);
  Stats.create("readouts.monitor", Counters.ReadoutsMonitor);
  Stats.create("readouts.timer_wraps", Counters.ReadoutsTimerWraps);

  Stats.create("readouts.error_version", Counters.ReadoutsErrorVersion);
  Stats.create("readouts.error_bytes", Counters.ReadoutsErrorBytes);
  Stats.create("readouts.seq_errors", Counters.ReadoutsSeqErrors);

  Stats.create("thread.processing_idle", Counters.ProcessingIdle);

  Stats.create("events.count", Counters.Events);
  Stats.create("events.geometry_errors", Counters.GeometryErrors);
  Stats.create("events.no_coincidence", Counters.EventsNoCoincidence);
  Stats.create("events.matched_clusters", Counters.EventsMatchedClusters);
  Stats.create("events.strip_gaps", Counters.EventsInvalidStripGap);
  Stats.create("events.wire_gaps", Counters.EventsInvalidWireGap);

  Stats.create("transmit.bytes", Counters.TxBytes);

  /// \todo below stats are common to all detectors and could/should be moved
  Stats.create("kafka.produce_fails", Counters.kafka_produce_fails);
  Stats.create("kafka.ev_errors", Counters.kafka_ev_errors);
  Stats.create("kafka.ev_others", Counters.kafka_ev_others);
  Stats.create("kafka.dr_errors", Counters.kafka_dr_errors);
  Stats.create("kafka.dr_others", Counters.kafka_dr_noerrors);

  Stats.create("memory.hitvec_storage.alloc_count", HitVectorStorage::Pool->Stats.AllocCount);
  Stats.create("memory.hitvec_storage.alloc_bytes", HitVectorStorage::Pool->Stats.AllocBytes);
  Stats.create("memory.hitvec_storage.dealloc_count", HitVectorStorage::Pool->Stats.DeallocCount);
  Stats.create("memory.hitvec_storage.dealloc_bytes", HitVectorStorage::Pool->Stats.DeallocBytes);
  Stats.create("memory.hitvec_storage.malloc_fallback_count", HitVectorStorage::Pool->Stats.MallocFallbackCount);

  Stats.create("memory.cluster_storage.alloc_count", ClusterPoolStorage::Pool->Stats.AllocCount);
  Stats.create("memory.cluster_storage.alloc_bytes", ClusterPoolStorage::Pool->Stats.AllocBytes);
  Stats.create("memory.cluster_storage.dealloc_count", ClusterPoolStorage::Pool->Stats.DeallocCount);
  Stats.create("memory.cluster_storage.dealloc_bytes", ClusterPoolStorage::Pool->Stats.DeallocBytes);
  Stats.create("memory.cluster_storage.malloc_fallback_count", ClusterPoolStorage::Pool->Stats.MallocFallbackCount);

  // clang-format on

  std::function<void()> inputFunc = [this]() { CAENBase::input_thread(); };
  Detector::AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    CAENBase::processing_thread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d Multiblade Rx ringbuffers of size %d",
         EthernetBufferMaxEntries, EthernetBufferSize);
}

void CAENBase::input_thread() {
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(),
                         EFUSettings.DetectorPort);
  UDPReceiver receiver(local);
  receiver.setBufferSizes(EFUSettings.TxSocketBufferSize,
                          EFUSettings.RxSocketBufferSize);
  receiver.checkRxBufferSizes(EFUSettings.RxSocketBufferSize);
  receiver.printBufferSizes();
  receiver.setRecvTimeout(0, 100000); /// secs, usecs 1/10s

  for (;;) {
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

    // Checking for exit
    if (not runThreads) {
      XTRACE(INPUT, ALW, "Stopping input thread.");
      return;
    }
  }
}

void CAENBase::processing_thread() {

  MBCaenInstrument MBCaen(Counters, EFUSettings, MBCAENSettings);

  // Event producer
  Producer eventprod(EFUSettings.KafkaBroker, MBCaen.topic);
  auto Produce = [&eventprod](auto DataBuffer, auto Timestamp) {
    eventprod.produce(DataBuffer, Timestamp);
  };
  EV42Serializer flatbuffer{KafkaBufferSize, "multiblade", Produce};

  unsigned int data_index;
  TSCTimer produce_timer(EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ);
  Timer h5flushtimer;
  // Monitor these counters
  RuntimeStat RtStat({Counters.RxPackets, Counters.Events, Counters.TxBytes});

  while (true) {
    if (InputFifo.pop(data_index)) { // There is data in the FIFO - do processing
      auto datalen = RxRingbuffer.getDataLength(data_index);
      if (datalen == 0) {
        Counters.FifoSeqErrors++;
        continue;
      }

      /// \todo use the Buffer<T> class here and in parser
      auto dataptr = RxRingbuffer.getDataBuffer(data_index);

      uint64_t efu_time = 1000000000LU * (uint64_t)time(NULL); // ns since 1970
      flatbuffer.pulseTime(efu_time);

      if (not MBCaen.parsePacket(dataptr, datalen, flatbuffer)) {
        continue;
      }

      auto cassette = MBCaen.MultibladeConfig.Mappings->cassette(MBCaen.parser.MBHeader->digitizerID);
      for (const auto &e : MBCaen.builders[cassette].Events) {

        if (!e.both_planes()) {
          Counters.EventsNoCoincidence++;
          continue;
        }

        bool DiscardGap{true};
        // Discard if there are gaps in the strip channels
        if (DiscardGap) {
          if (e.ClusterB.hits.size() < e.ClusterB.coord_span()) {
            Counters.EventsInvalidStripGap++;
            continue;
          }
        }

        // Discard if there are gaps in the wire channels
        if (DiscardGap) {
          if (e.ClusterA.hits.size() < e.ClusterA.coord_span()) {
            Counters.EventsInvalidWireGap++;
            continue;
          }
        }

        Counters.EventsMatchedClusters++;

        XTRACE(EVENT, INF, "Event Valid\n %s", e.to_string({}, true).c_str());
        // calculate local x and y using center of mass
        auto x = static_cast<uint16_t>(std::round(e.ClusterA.coord_center()));
        auto y = static_cast<uint16_t>(std::round(e.ClusterB.coord_center()));

        // \todo improve this
        auto time = e.time_start();
        auto pixel_id = MBCaen.essgeom.pixel2D(x, y);
        XTRACE(EVENT, DEB, "time: %u, x %u, y %u, pixel %u", time, x, y, pixel_id);

        if (pixel_id == 0) {
          Counters.GeometryErrors++;
        } else {
          Counters.TxBytes += flatbuffer.addEvent(time, pixel_id);
          Counters.Events++;
        }
      }
      MBCaen.builders[cassette].Events.clear(); // else events will accumulate
    } else {
      // There is NO data in the FIFO - do stop checks and sleep a little
      Counters.ProcessingIdle++;
      usleep(10);
    }

    // if filedumping and requesting time splitting, check for rotation.
    if (MBCAENSettings.H5SplitTime != 0 and (MBCaen.dumpfile)) {
      if (h5flushtimer.timeus() >= MBCAENSettings.H5SplitTime * 1000000) {

        /// \todo user should not need to call flush() - implicit in rotate() ?
        MBCaen.dumpfile->flush();
        MBCaen.dumpfile->rotate();
        h5flushtimer.reset();
      }
    }

    if (produce_timer.timeout()) {

      RuntimeStatusMask =  RtStat.getRuntimeStatusMask({Counters.RxPackets, Counters.Events, Counters.TxBytes});

      Counters.TxBytes += flatbuffer.produce();

      if (!MBCaen.histograms.isEmpty()) {
        // XTRACE(PROCESS, INF, "Sending histogram for %zu readouts",
        //   histograms.hit_count());
        MBCaen.histfb.produce(MBCaen.histograms);
        MBCaen.histograms.clear();
      }

      /// Kafka stats update - common to all detectors
      /// don't increment as producer keeps absolute count
      Counters.kafka_produce_fails = eventprod.stats.produce_fails;
      Counters.kafka_ev_errors = eventprod.stats.ev_errors;
      Counters.kafka_ev_others = eventprod.stats.ev_others;
      Counters.kafka_dr_errors = eventprod.stats.dr_errors;
      Counters.kafka_dr_noerrors = eventprod.stats.dr_noerrors;
    }

    if (not runThreads) {
      // \todo flush everything here
      XTRACE(INPUT, ALW, "Stopping processing thread.");
      return;
    }
  }
}

}
