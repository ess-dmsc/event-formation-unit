// Copyright (C) 2018-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Multi-Blade prototype detector base plugin interface definition
///
//===----------------------------------------------------------------------===//

#include <multiblade/MBCaenBase.h>
#include <cinttypes>
#include <common/EFUArgs.h>
#include <common/RingBuffer.h>
#include <common/TimeString.h>
#include <common/TestImageUdder.h>
#include <unistd.h>
#include <common/SPSCFifo.h>
#include <common/Socket.h>
#include <common/TSCTimer.h>
#include <common/Timer.h>
#include <multiblade/MBCaenInstrument.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Multiblade {

using namespace memory_sequential_consistent; // Lock free fifo

const char *classname = "Multiblade detector with CAEN readout";

const int TSC_MHZ = 2900; // MJC's workstation - not reliable


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

  Stats.create("readouts.error_version", Counters.ReadoutsErrorVersion);
  Stats.create("readouts.error_bytes", Counters.ReadoutsErrorBytes);
  Stats.create("readouts.seq_errors", Counters.ReadoutsSeqErrors);

  Stats.create("thread.processing_idle", Counters.ProcessingIdle);

  Stats.create("events.count", Counters.Events);
  Stats.create("events.udder", Counters.EventsUdder);
  Stats.create("events.geometry_errors", Counters.GeometryErrors);
  Stats.create("events.no_coincidence", Counters.EventsNoCoincidence);
  Stats.create("events.not_adjacent", Counters.EventsNotAdjacent);
  Stats.create("filters.max_time_span", Counters.FiltersMaxTimeSpan);
  Stats.create("filters.max_multi1", Counters.FiltersMaxMulti1);
  Stats.create("filters.max_multi2", Counters.FiltersMaxMulti2);

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
  /// \todo the number 11 is a workaround
  EthernetRingbuffer = new RingBuffer<EthernetBufferSize>(EthernetBufferMaxEntries + 11);
  assert(EthernetRingbuffer != 0);
}

void CAENBase::input_thread() {
  /** Connection setup */
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(),
                         EFUSettings.DetectorPort);
  UDPReceiver receiver(local);
  // receiver.buflen(opts->buflen);
  receiver.setBufferSizes(EFUSettings.TxSocketBufferSize,
                          EFUSettings.RxSocketBufferSize);
  receiver.checkRxBufferSizes(EFUSettings.RxSocketBufferSize);
  receiver.printBufferSizes();
  receiver.setRecvTimeout(0, 100000); /// secs, usecs 1/10s

  for (;;) {
    int rdsize;
    unsigned int eth_index = EthernetRingbuffer->getDataIndex();

    /** this is the processing step */
    EthernetRingbuffer->setDataLength(eth_index, 0);
    if ((rdsize = receiver.receive(EthernetRingbuffer->getDataBuffer(eth_index),
                                   EthernetRingbuffer->getMaxBufSize())) > 0) {
      EthernetRingbuffer->setDataLength(eth_index, rdsize);
      XTRACE(INPUT, DEB, "Received an udp packet of length %d bytes", rdsize);
      Counters.RxPackets++;
      Counters.RxBytes += rdsize;

      if (InputFifo.push(eth_index) == false) {
        Counters.FifoPushErrors++;
      } else {
        EthernetRingbuffer->getNextBuffer();
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


  if (EFUSettings.TestImage) {
    XTRACE(PROCESS, ALW, "GENERATING TEST IMAGE!");
    Udder udder;
    uint32_t time_of_flight = 0;
    while (true) {
      if (not runThreads) {
        // \todo flush everything here
        XTRACE(INPUT, ALW, "Stopping processing thread.");
        return;
      }

      static int eventCount = 0;
      if (eventCount == 0) {
        uint64_t efu_time = 1000000000LU * (uint64_t)time(NULL); // ns since 1970
        flatbuffer.pulseTime(efu_time);
      }

      auto pixel_id = udder.getPixel(MBCaen.essgeom.nx(), MBCaen.essgeom.ny(), &MBCaen.essgeom);

      Counters.TxBytes += flatbuffer.addEvent(time_of_flight, pixel_id);
      Counters.EventsUdder++;

      if (EFUSettings.TestImageUSleep != 0) {
        usleep(EFUSettings.TestImageUSleep);
      }

      time_of_flight++;

      if (Counters.TxBytes != 0) {
        eventCount = 0;
      } else {
        eventCount++;
      }
    }
  }


  unsigned int data_index;
  TSCTimer produce_timer;
  Timer h5flushtimer;
  while (true) {
    if (InputFifo.pop(data_index)) { // There is data in the FIFO - do processing
      auto datalen = EthernetRingbuffer->getDataLength(data_index);
      if (datalen == 0) {
        Counters.FifoSeqErrors++;
        continue;
      }

      /// \todo use the Buffer<T> class here and in parser
      auto dataptr = EthernetRingbuffer->getDataBuffer(data_index);

      if (not MBCaen.parsePacket(dataptr, datalen, flatbuffer)) {
        continue;
      }

      auto cassette = MBCaen.MultibladeConfig.Mappings->cassette(MBCaen.parser.MBHeader->digitizerID);
      for (const auto &e : MBCaen.builders[cassette].Events) {
        if (MBCaen.filterEvent(e)) {
          continue;
        }

        XTRACE(EVENT, INF, "Event Valid\n %s", e.to_string({}, true).c_str());
        // calculate local x and y using center of mass
        auto x = static_cast<uint16_t>(std::round(e.ClusterA.coord_center()));
        auto y = static_cast<uint16_t>(std::round(e.ClusterB.coord_center()));

        // \todo improve this
        auto time = e.time_start() * MBCaen.MultibladeConfig.TimeTickNS; // TOF in ns
        auto pixel_id = MBCaen.essgeom.pixel2D(x, y);
        XTRACE(EVENT, DEB, "time: %u, x %u, y %u, pixel %u", time, x, y, pixel_id);

        if (pixel_id == 0) {
          Counters.GeometryErrors++;
        } else {
          Counters.TxBytes += flatbuffer.addEvent(time, pixel_id);
          Counters.Events++;
        }
      }
      MBCaen.builders[cassette].Events.clear();
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
        h5flushtimer.now();
      }
    }

    if (produce_timer.timetsc() >=
        EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ) {

      Counters.TxBytes += flatbuffer.produce();

      if (!MBCaen.histograms.isEmpty()) {
//        XTRACE(PROCESS, INF, "Sending histogram for %zu readouts",
//               histograms.hit_count());
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

      produce_timer.now();
    }

    if (not runThreads) {
      // \todo flush everything here
      XTRACE(INPUT, ALW, "Stopping processing thread.");
      return;
    }
  }
}

}
