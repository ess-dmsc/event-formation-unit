/** Copyright (C) 2017-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Implementation of the detector pipeline plugin for MUlti-Blade
/// detectors.
/// Contributor: Carsten Søgaard, Niels Bohr Institute, University of Copenhagen
//===----------------------------------------------------------------------===//

#include "MBCaenBase.h"

#include <cinttypes>
#include <common/EFUArgs.h>
#include <common/EV42Serializer.h>
#include <common/Producer.h>
#include <common/HistSerializer.h>
#include <common/RingBuffer.h>
#include <common/Trace.h>
#include <common/TimeString.h>
#include <common/TestImageUdder.h>

#include <unistd.h>

#include <libs/include/SPSCFifo.h>
#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>

#include <caen/DataParser.h>

#include <clustering/EventBuilder2.h>

#include <logical_geometry/ESSGeometry.h>
#include <caen/MBGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_WAR

namespace Multiblade {

using namespace memory_sequential_consistent; // Lock free fifo

const char *classname = "Multiblade detector with CAEN readout";

const int TSC_MHZ = 2900; // MJC's workstation - not reliable


CAENBase::CAENBase(BaseSettings const &settings, struct CAENSettings &LocalMBCAENSettings)
    : Detector("MBCAEN", settings), MBCAENSettings(LocalMBCAENSettings) {
  Stats.setPrefix("efu.mbcaen");

  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off
  Stats.create("input.rx_packets", mystats.rx_packets);
  Stats.create("input.rx_bytes", mystats.rx_bytes);
  Stats.create("input.fifo1_push_errors", mystats.fifo1_push_errors);
  Stats.create("processing.rx_readouts", mystats.rx_readouts);
  Stats.create("processing.rx_error_bytes", mystats.rx_error_bytes);
  Stats.create("processing.rx_seq_errors", mystats.rx_seq_errors);
  Stats.create("processing.rx_idle1", mystats.rx_idle1);
  Stats.create("processing.tx_bytes", mystats.tx_bytes);
  Stats.create("processing.rx_events", mystats.rx_events);
  Stats.create("processing.rx_geometry_errors", mystats.geometry_errors);
  Stats.create("processing.fifo_seq_errors", mystats.fifo_seq_errors);
  /// \todo below stats are common to all detectors and could/should be moved
  Stats.create("kafka_produce_fails", mystats.kafka_produce_fails);
  Stats.create("kafka_ev_errors", mystats.kafka_ev_errors);
  Stats.create("kafka_ev_others", mystats.kafka_ev_others);
  Stats.create("kafka_dr_errors", mystats.kafka_dr_errors);
  Stats.create("kafka_dr_others", mystats.kafka_dr_noerrors);
  // clang-format on

  std::function<void()> inputFunc = [this]() { CAENBase::input_thread(); };
  Detector::AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    CAENBase::processing_thread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d Multiblade Rx ringbuffers of size %d",
         eth_buffer_max_entries, eth_buffer_size);
  /// \todo the number 11 is a workaround
  eth_ringbuf = new RingBuffer<eth_buffer_size>(eth_buffer_max_entries + 11);
  assert(eth_ringbuf != 0);

  mb_opts = Config(MBCAENSettings.ConfigFile);
  assert(mb_opts.getDigitizers() != nullptr);
}

void CAENBase::input_thread() {
  /** Connection setup */
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(),
                         EFUSettings.DetectorPort);
  UDPReceiver receiver(local);
  // receiver.buflen(opts->buflen);
  receiver.setBufferSizes(0, EFUSettings.DetectorRxBufferSize);
  receiver.printBufferSizes();
  receiver.setRecvTimeout(0, 100000); /// secs, usecs 1/10s

  int rdsize;
  for (;;) {
    unsigned int eth_index = eth_ringbuf->getDataIndex();

    /** this is the processing step */
    eth_ringbuf->setDataLength(eth_index, 0);
    if ((rdsize = receiver.receive(eth_ringbuf->getDataBuffer(eth_index),
                                   eth_ringbuf->getMaxBufSize())) > 0) {
      eth_ringbuf->setDataLength(eth_index, rdsize);
//      XTRACE(INPUT, DEB, "Received an udp packet of length %d bytes",
//             rdsize);
      mystats.rx_packets++;
      mystats.rx_bytes += rdsize;

      if (input2proc_fifo.push(eth_index) == false) {
        mystats.fifo1_push_errors++;
      } else {
        eth_ringbuf->getNextBuffer();
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
  // \todo get from opts?
  bool time_span_filter = false;
  bool filter_multiplicity = false;
  bool filter_multiplicity2 = false;

  const uint16_t ncass = mb_opts.getCassettes();
  const uint16_t nwires = mb_opts.getWires();
  const uint16_t nstrips = mb_opts.getStrips();
  std::string topic{""};
  std::string monitor{""};

  MBGeometry mbgeom(ncass, nwires, nstrips);
  ESSGeometry essgeom;
  if (mb_opts.getInstrument() == Config::InstrumentGeometry::Estia) {
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

  if (mb_opts.getDetectorType() == Config::DetectorType::MB18) {
    XTRACE(PROCESS, ALW, "Setting detector to MB18");
    mbgeom.setDetectorMB18();
  } else {
    XTRACE(PROCESS, ALW, "Setting detector to MB16");
    mbgeom.setDetectorMB16();
  }

  std::shared_ptr<ReadoutFile> dumpfile;
  if (!MBCAENSettings.FilePrefix.empty()) {
    dumpfile = ReadoutFile::create(
        MBCAENSettings.FilePrefix + "-" + timeString());
  }

  EV42Serializer flatbuffer(kafka_buffer_size, "multiblade");
  Producer eventprod(EFUSettings.KafkaBroker, topic);
  flatbuffer.setProducerCallback(
      std::bind(&Producer::produce2<uint8_t>, &eventprod, std::placeholders::_1));

  Hists histograms(std::max(ncass * nwires, ncass * nstrips), 65535);
  Producer monitorprod(EFUSettings.KafkaBroker, monitor);
  HistSerializer histfb(histograms.needed_buffer_size());
  histfb.set_callback(
      std::bind(&Producer::produce2<uint8_t>, &monitorprod, std::placeholders::_1));

  std::vector<EventBuilder2> builders(ncass);

  DataParser parser;
  auto digitisers = mb_opts.getDigitisers();
  DigitizerMapping mb1618(digitisers);

  Udder udder;
  unsigned int data_index;
  TSCTimer produce_timer;
  Timer h5flushtimer;
  while (true) {
    if (input2proc_fifo.pop(data_index)) { // There is data in the FIFO - do processing
      auto datalen = eth_ringbuf->getDataLength(data_index);
      if (datalen == 0) {
        mystats.fifo_seq_errors++;
        continue;
      }

      /// \todo use the Buffer<T> class here and in parser
      auto dataptr = eth_ringbuf->getDataBuffer(data_index);
      if (parser.parse(dataptr, datalen) < 0) {
        mystats.rx_error_bytes += parser.Stats.error_bytes;
        continue;
      }
      mystats.rx_seq_errors += parser.Stats.seq_errors;

      XTRACE(DATA, DEB, "Received %d readouts from digitizer %d",
             parser.MBHeader->numElements, parser.MBHeader->digitizerID);

      mystats.rx_readouts += parser.MBHeader->numElements;

      if (dumpfile) {
        dumpfile->push(parser.readouts);
      }

      /// \todo why can't I use mb_opts.detector->cassette()
      auto cassette = mb1618.cassette(parser.MBHeader->digitizerID);
      if (cassette < 0) {
        XTRACE(DATA, WAR, "Invalid digitizerId: %d",
               parser.MBHeader->digitizerID);
        continue;
      }

      for (const auto &dp : parser.readouts) {
        // \todo should there not be a function in mbgeom for validating this?
        assert(dp.channel < nwires + nstrips);

        uint16_t __attribute__((unused)) coord; // \todo invalidate ?
        uint8_t plane = mbgeom.getPlane(dp.channel);

        // \todo same as above, geometry should validate this
        assert(plane < 2);

        if (plane == 0) {
          coord = mbgeom.getx(cassette, dp.channel);
          histograms.bin_x(coord, dp.adc);
        }

        if (plane == 1) {
          coord = mbgeom.gety(cassette, dp.channel);
          histograms.bin_y(coord, dp.adc);
        }

        builders[cassette].insert({dp.local_time, coord, dp.adc, plane});

        XTRACE(DATA, DEB, "Readout (%s) -> cassette=%d plane=%d coord=%d",
               dp.debug().c_str(), cassette, plane, coord);
      }

      builders[cassette].flush();
      for (const auto &e : builders[cassette].matcher.matched_events) {
        if (!e.both_planes())
          continue;

        // \todo parametrize maximum time span - in opts?
        if (time_span_filter && (e.time_span() > 313)) {
          // \todo add counter
          continue;
        }

        // \todo are these always wires && strips respectively?
        if (filter_multiplicity &&
            ((e.c1.hit_count() > 5) || (e.c2.hit_count() > 10))) {
          // \todo add counter
          continue;
        }
        if (filter_multiplicity2 &&
            ((e.c1.hit_count() > 3) || (e.c2.hit_count() > 4))) {
          // \todo add counter
          continue;
        }

        XTRACE(DATA, DEB, "Event\n %s", e.debug(true).c_str());
        // calculate local x and y using center of mass
        auto localx = static_cast<uint16_t>(std::round(e.c1.coord_center()));
        auto localy = static_cast<uint16_t>(std::round(e.c2.coord_center()));

        // calculate local x and y using center of span
//        auto localx = (e.c1.coord_start() + e.c1.coord_end()) / 2;
//        auto localy = (e.c2.coord_start() + e.c2.coord_end()) / 2;

        // \todo improve this
        auto time = e.time_start();
        // \todo use essgeom
        auto pixel_id = essgeom.pixel2D(localx, localy);
        if (pixel_id == 0) {
          mystats.geometry_errors++;
        } else {
          mystats.tx_bytes += flatbuffer.addEvent(time, pixel_id);
          mystats.rx_events++;
        }
      }
      builders[cassette].matcher.matched_events.clear();


#if 0
      static uint32_t time = 0;
      auto pixel_id = TestImage2D(&essgeom);

      if (pixel_id == 0) {
        mystats.geometry_errors++;
      } else {
        mystats.tx_bytes += flatbuffer.addEvent(time, pixel_id);
        mystats.rx_events++;
      }
      time++;
#endif

    } else {
      // There is NO data in the FIFO - do stop checks and sleep a little
      mystats.rx_idle1++;
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

      mystats.tx_bytes += flatbuffer.produce();

      if (!histograms.isEmpty()) {
//        XTRACE(PROCESS, INF, "Sending histogram for %zu readouts",
//               histograms.hit_count());
        histfb.produce(histograms);
        histograms.clear();
      }

      /// Kafka stats update - common to all detectors
      /// don't increment as producer keeps absolute count
      mystats.kafka_produce_fails = eventprod.stats.produce_fails;
      mystats.kafka_ev_errors = eventprod.stats.ev_errors;
      mystats.kafka_ev_others = eventprod.stats.ev_others;
      mystats.kafka_dr_errors = eventprod.stats.dr_errors;
      mystats.kafka_dr_noerrors = eventprod.stats.dr_noerrors;

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
