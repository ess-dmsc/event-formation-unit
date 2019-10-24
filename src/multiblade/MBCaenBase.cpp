/** Copyright (C) 2017-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Implementation of the detector pipeline plugin for MUlti-Blade
/// detectors.
//===----------------------------------------------------------------------===//

#include "MBCaenBase.h"

#include <cinttypes>
#include <common/EFUArgs.h>
#include <common/EV42Serializer.h>
#include <common/Producer.h>
#include <common/monitor/HistogramSerializer.h>
#include <common/RingBuffer.h>
#include <common/Trace.h>
#include <common/TimeString.h>
#include <common/TestImageUdder.h>

#include <unistd.h>

#include <common/SPSCFifo.h>
#include <common/Socket.h>
#include <common/TSCTimer.h>
#include <common/Timer.h>

#include <caen/DataParser.h>

#include <clustering/EventBuilder.h>

#include <logical_geometry/ESSGeometry.h>
#include <caen/MBGeometry.h>

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
  Stats.create("receive.dropped", Counters.FifoPushErrors);
  Stats.create("receive.fifo_seq_errors", Counters.FifoSeqErrors);

  Stats.create("readouts.count", Counters.ReadoutsCount);
  Stats.create("readouts.count_valid", Counters.ReadoutsGood);
  Stats.create("readouts.invalid_ch", Counters.ReadoutsInvalidChannel);
  Stats.create("readouts.invalid_adc", Counters.ReadoutsInvalidAdc);
  Stats.create("readouts.invalid_plane", Counters.ReadoutsInvalidPlane);
  Stats.create("readouts.monitor", Counters.ReadoutsMonitor);

  Stats.create("readouts.error_version", Counters.ReadoutsErrorVersion);
  Stats.create("readouts.error_bytes", Counters.ReadoutsErrorBytes);
  Stats.create("readouts.seq_errors", Counters.ReadoutsSeqErrors);

  Stats.create("thread.processing_idle", Counters.RxIdle);

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

  MultibladeConfig = Config(MBCAENSettings.ConfigFile);
  assert(MultibladeConfig.getDigitizers() != nullptr);
}

void CAENBase::input_thread() {
  /** Connection setup */
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(),
                         EFUSettings.DetectorPort);
  UDPReceiver receiver(local);
  // receiver.buflen(opts->buflen);
  receiver.setBufferSizes(0, EFUSettings.DetectorRxBufferSize);
  receiver.checkRxBufferSizes(EFUSettings.DetectorRxBufferSize);
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
    }

    // Checking for exit
    if (not runThreads) {
      XTRACE(INPUT, ALW, "Stopping input thread.");
      return;
    }
  }
}

void CAENBase::processing_thread() {
  const uint16_t ncass = MultibladeConfig.getCassettes();
  const uint16_t nwires = MultibladeConfig.getWires();
  const uint16_t nstrips = MultibladeConfig.getStrips();
  std::string topic{""};
  std::string monitor{""};

  MBGeometry mbgeom(ncass, nwires, nstrips);
  ESSGeometry essgeom;
  if (MultibladeConfig.getInstrument() == Config::InstrumentGeometry::Estia) {
    XTRACE(PROCESS, ALW, "Setting instrument configuration to Estia");
    mbgeom.setConfigurationEstia();
    essgeom = ESSGeometry(ncass * nwires, nstrips, 1, 1);
    topic = "ESTIA_detector";
    monitor = "ESTIA_monitor";
  } else {
    mbgeom.setConfigurationFreia();
    XTRACE(PROCESS, ALW, "Setting instrument configuration to Freia");
    essgeom = ESSGeometry(nstrips, ncass * nwires, 1, 1);
    topic = "FREIA_detector";
    monitor = "FREIA_monitor";
  }

  if (MultibladeConfig.getDetectorType() == Config::DetectorType::MB18) {
    XTRACE(PROCESS, ALW, "Setting detector to MB18");
    mbgeom.setDetectorMB18();
  } else {
    XTRACE(PROCESS, ALW, "Setting detector to MB16");
    mbgeom.setDetectorMB16();
  }

  std::shared_ptr<ReadoutFile> dumpfile;
  if (!MBCAENSettings.FilePrefix.empty()) {
    dumpfile = ReadoutFile::create(MBCAENSettings.FilePrefix + "-" + timeString());
  }

  EV42Serializer flatbuffer(KafkaBufferSize, "multiblade");
  Producer eventprod(EFUSettings.KafkaBroker, topic);
#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
  flatbuffer.setProducerCallback(
      std::bind(&Producer::produce2<uint8_t>, &eventprod, std::placeholders::_1));

  Hists histograms(std::max(ncass * nwires, ncass * nstrips), 65535);
  Producer monitorprod(EFUSettings.KafkaBroker, monitor);
  HistogramSerializer histfb(histograms.needed_buffer_size(), "multiblade");
  histfb.set_callback(
      std::bind(&Producer::produce2<uint8_t>, &monitorprod, std::placeholders::_1));
#pragma GCC diagnostic pop
  std::vector<EventBuilder> builders(ncass);

  DataParser parser;
  auto digitisers = MultibladeConfig.getDigitisers();
  DigitizerMapping mb1618(digitisers);


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

      auto pixel_id = udder.getPixel(essgeom.nx(), essgeom.ny(), &essgeom);

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
      if (parser.parse(dataptr, datalen) < 0) {
        Counters.ReadoutsErrorBytes += parser.Stats.error_bytes;
        Counters.ReadoutsErrorVersion += parser.Stats.error_version;
        continue;
      }
      Counters.ReadoutsSeqErrors += parser.Stats.seq_errors;

      XTRACE(DATA, DEB, "Received %d readouts from digitizer %d",
             parser.MBHeader->numElements, parser.MBHeader->digitizerID);

      uint64_t efu_time = 1000000000LU * (uint64_t)time(NULL); // ns since 1970
      flatbuffer.pulseTime(efu_time);

      Counters.ReadoutsCount += parser.MBHeader->numElements;

      if (dumpfile) {
        dumpfile->push(parser.readouts);
      }

      /// \todo why can't I use MultibladeConfig.detector->cassette()
      auto cassette = mb1618.cassette(parser.MBHeader->digitizerID);
      if (cassette < 0) {
        XTRACE(DATA, WAR, "Invalid digitizerId: %d",
               parser.MBHeader->digitizerID);
        continue;
      }

      for (const auto &dp : parser.readouts) {

        if (not mbgeom.isValidCh(dp.channel)) {
          Counters.ReadoutsInvalidChannel++;
          continue;
        }

        if (dp.adc > MultibladeConfig.max_valid_adc) {
          Counters.ReadoutsInvalidAdc++;
          continue;
        }

        uint8_t plane = mbgeom.getPlane(dp.channel);
        uint16_t global_ch = mbgeom.getGlobalChannel(cassette, dp.channel);
        uint16_t coord;
        if (plane == 0) {
          if (global_ch == 30) {
            Counters.ReadoutsMonitor++;
            continue;
          }
          coord = mbgeom.getx(cassette, dp.channel);
          histograms.bin_x(global_ch, dp.adc);
        } else  if (plane == 1) {
          coord = mbgeom.gety(cassette, dp.channel);
          histograms.bin_y(global_ch, dp.adc);
        } else {
          Counters.ReadoutsInvalidPlane++;
          continue;
        }

        Counters.ReadoutsGood++;

        XTRACE(DATA, DEB, "time %lu, channel %u, adc %u", dp.local_time, dp.channel, dp.adc);

        builders[cassette].insert({dp.local_time, coord, dp.adc, plane});

        XTRACE(DATA, DEB, "Readout (%s) -> cassette=%d plane=%d coord=%d",
               dp.debug().c_str(), cassette, plane, coord);
      }

      builders[cassette].flush();
      for (const auto &e : builders[cassette].matcher.matched_events) {

        if (!e.both_planes()) {
          XTRACE(EVENT, INF, "Event No Coincidence %s", e.to_string({}, true).c_str());
          Counters.EventsNoCoincidence++;
          continue;
        }

        // \todo parametrize maximum time span - in opts?
        if (MultibladeConfig.filter_time_span && (e.time_span() > MultibladeConfig.filter_time_span_value)) {
          XTRACE(EVENT, INF, "Event filter time_span %s", e.to_string({}, true).c_str());
          Counters.FiltersMaxTimeSpan++;
          continue;
        }

        if ((e.ClusterA.coord_span() > e.ClusterA.hit_count()) && (e.ClusterB.coord_span() > e.ClusterB.hit_count())) {
          XTRACE(EVENT, INF, "Event Chs not adjacent %s", e.to_string({}, true).c_str());
          Counters.EventsNotAdjacent++;
          continue;
        }

        // // \todo are these always wires && strips respectively?
        // if (filter_multiplicity &&
        //     ((e.cluster1.hit_count() > 5) || (e.cluster2.hit_count() > 10))) {
        //   Counters.FiltersMaxMulti1++;
        //   continue;
        // }
        // if (filter_multiplicity2 &&
        //     ((e.cluster1.hit_count() > 3) || (e.cluster2.hit_count() > 4))) {
        //   Counters.FiltersMaxMulti2++;
        //   continue;
        // }

        XTRACE(EVENT, INF, "Event Valid\n %s", e.to_string({}, true).c_str());
        // calculate local x and y using center of mass
        auto x = static_cast<uint16_t>(std::round(e.ClusterA.coord_center()));
        auto y = static_cast<uint16_t>(std::round(e.ClusterB.coord_center()));

        // calculate local x and y using center of span
//        auto x = (e.cluster1.coord_start() + e.cluster1.coord_end()) / 2;
//        auto y = (e.cluster2.coord_start() + e.cluster2.coord_end()) / 2;

        // \todo improve this
        auto time = e.time_start() * MultibladeConfig.TimeTickNS; // TOF in ns
        auto pixel_id = essgeom.pixel2D(x, y);
        XTRACE(EVENT, DEB, "time: %u, x %u, y %u, pixel %u", time, x, y, pixel_id);

        if (pixel_id == 0) {
          Counters.GeometryErrors++;
        } else {
          Counters.TxBytes += flatbuffer.addEvent(time, pixel_id);
          Counters.Events++;
        }
      }

    } else {
      // There is NO data in the FIFO - do stop checks and sleep a little
      Counters.RxIdle++;
      usleep(10);
    }

    // if filedumping and requesting time splitting, check for rotation.
    if (MBCAENSettings.H5SplitTime != 0 and (dumpfile)) {
      if (h5flushtimer.timeus() >= MBCAENSettings.H5SplitTime * 1000000) {

        /// \todo user should not need to call flush() - implicit in rotate() ?
        dumpfile->flush();
        dumpfile->rotate();
        h5flushtimer.now();
      }
    }

    if (produce_timer.timetsc() >=
        EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ) {

      Counters.TxBytes += flatbuffer.produce();

      if (!histograms.isEmpty()) {
//        XTRACE(PROCESS, INF, "Sending histogram for %zu readouts",
//               histograms.hit_count());
        histfb.produce(histograms);
        histograms.clear();
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
