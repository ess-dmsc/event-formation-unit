/** Copyright (C) 2017-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Implementation of the detector pipeline plugin for Jalousie
/// detectors.
//===----------------------------------------------------------------------===//

#include <jalousie/JalousieBase.h>
#include <jalousie/Readout.h>
#include <common/EFUArgs.h>
#include <common/EV42Serializer.h>
#include <common/Producer.h>
#include <common/HistSerializer.h>
#include <common/RingBuffer.h>
#include <common/TimeString.h>
#include <common/TestImageUdder.h>

#include <common/SPSCFifo.h>
#include <common/Socket.h>
#include <common/TSCTimer.h>
#include <common/Timer.h>

#include <logical_geometry/ESSGeometry.h>
#include <common/reduction/ChronoMerger.h>
#include "JalousieBase.h"

#include <common/Trace.h>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

#include <common/Log.h>
//#undef TRC_MASK
//#define TRC_MASK 0

namespace Jalousie {

using namespace memory_sequential_consistent; // Lock free fifo

const char *classname = "Jalousie detector";

const int TSC_MHZ = 2900; // MJC's workstation - not reliable


JalousieBase::JalousieBase(BaseSettings const &settings, CLISettings const &LocalSettings)
    : Detector("JALOUSIE", settings), ModuleSettings(LocalSettings) {

  Stats.setPrefix(EFUSettings.GraphitePrefix, EFUSettings.GraphiteRegion);

  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off
  Stats.create("receive.packets", mystats.rx_packets);
  Stats.create("receive.bytes", mystats.rx_bytes);
  Stats.create("receive.dropped", mystats.fifo_push_errors);

  Stats.create("readouts.count", mystats.readout_count);
  Stats.create("readouts.bad_module_id", mystats.bad_module_id);
  Stats.create("readouts.chopper_pulses", mystats.chopper_pulses);

  Stats.create("events.count", mystats.events);
  Stats.create("events.geometry_errors", mystats.geometry_errors);
  Stats.create("events.timing_errors", mystats.timing_errors);
  Stats.create("transmit.bytes", mystats.tx_bytes);

  Stats.create("thread.seq_errors", mystats.fifo_seq_errors);
  Stats.create("thread.processing_idle", mystats.processing_idle);

    /// \todo below stats are common to all detectors and could/should be moved
  Stats.create("kafka.produce_fails", mystats.kafka_produce_fails);
  Stats.create("kafka.ev_errors", mystats.kafka_ev_errors);
  Stats.create("kafka.ev_others", mystats.kafka_ev_others);
  Stats.create("kafka.dr_errors", mystats.kafka_dr_errors);
  Stats.create("kafka.dr_others", mystats.kafka_dr_noerrors);
  // clang-format on

  std::function<void()> inputFunc = [this]() { JalousieBase::input_thread(); };
  Detector::AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    JalousieBase::processing_thread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d Jalousie Rx ringbuffers of size %d",
         eth_buffer_max_entries, eth_buffer_size);
  /// \todo the number 11 is a workaround
  eth_ringbuf = new RingBuffer<eth_buffer_size>(eth_buffer_max_entries + 11);
  assert(eth_ringbuf != 0);
}

void JalousieBase::input_thread() {
  /** Connection setup */
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(),
                         EFUSettings.DetectorPort);
  UDPReceiver receiver(local);
  // receiver.buflen(opts->buflen);
  receiver.setBufferSizes(0, EFUSettings.DetectorRxBufferSize);
  receiver.printBufferSizes();
  receiver.setRecvTimeout(0, 100000); /// secs, usecs 1/10s

  for (;;) {
    int rdsize;
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
        mystats.fifo_push_errors++;
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

void JalousieBase::processing_thread() {
  LOG(INIT, Sev::Info, "Jalousie Config file: {}", ModuleSettings.ConfigFile);
  config = Config(ModuleSettings.ConfigFile);
  LOG(INIT, Sev::Info, "Jalousie Config\n{}", config.debug());

  constexpr size_t invalid_board_mapping {std::numeric_limits<size_t>::max()};

  std::string topic{""};
  std::string monitor{""};

  topic = "DREAM_detector";
  monitor = "DREAM_monitor";

  EV42Serializer flatbuffer(kafka_buffer_size, "DREAM_detector");
  Producer eventprod(EFUSettings.KafkaBroker, topic);
#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
  flatbuffer.setProducerCallback(
      std::bind(&Producer::produce2<uint8_t>, &eventprod, std::placeholders::_1));

  // TODO
  Hists histograms(65535, 65535);
  Producer monitorprod(EFUSettings.KafkaBroker, monitor);
  HistSerializer histfb(histograms.needed_buffer_size(), "jalousie");
  histfb.set_callback(
      std::bind(&Producer::produce2<uint8_t>, &monitorprod, std::placeholders::_1));
#pragma GCC diagnostic pop

  uint64_t previous_time{0}; /// < for timing error checks

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

      auto dataptr = eth_ringbuf->getDataBuffer(data_index);
      assert(datalen % sizeof(Jalousie::Readout) == 0);

      for (size_t i = 0; i < datalen / sizeof(Jalousie::Readout); i++) {
        auto rdout = (Jalousie::Readout *)dataptr;
//        printf("time %lu, board: %u, sub_id: %u, anode: %u, cathode: %u\n",
//           rdout->time, rdout->board, rdout->sub_id, rdout->anode, rdout->cathode);
        dataptr += sizeof(Jalousie::Readout);
        mystats.readout_count++;

        size_t module = config.board_mappings[rdout->board];

        if (module == invalid_board_mapping)
        {
          mystats.bad_module_id++;
          continue;
        }

        if (rdout->sub_id == Readout::chopper_sub_id) {
          mystats.chopper_pulses++;
          config.merger.insert(module, {rdout->time, 0});
        }
        else {
          auto pixel_id = config.geometry.pixelMP2D(rdout->anode, rdout->cathode, module);
          if (pixel_id == 0) {
            mystats.geometry_errors++;
          } else {
            config.merger.insert(module, {rdout->time, pixel_id});
            mystats.events++;
          }
        }
      }

      config.merger.sort();

      while (config.merger.ready()) {
        auto event = config.merger.pop_earliest();
        if (previous_time > event.time)
          mystats.timing_errors++;
        previous_time = event.time;

        if (event.pixel_id == 0) {
          // case of chopper pulse
          if (flatbuffer.eventCount()) {
            mystats.tx_bytes += flatbuffer.produce();
          }
          flatbuffer.pulseTime(event.time);
        } else {
          // regular neutron event
          uint32_t event_time = event.time - flatbuffer.pulseTime();
          mystats.tx_bytes += flatbuffer.addEvent(event_time, event.pixel_id);
        }
      }

    } else {
      // There is NO data in the FIFO - do stop checks and sleep a little
      mystats.processing_idle++;
      usleep(10);
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

      config.merger.sort();
      while (!config.merger.empty()) {
        auto event = config.merger.pop_earliest();
        if (previous_time > event.time)
          mystats.timing_errors++;
        previous_time = event.time;

        if (event.pixel_id == 0) {
          // chopper pulse
          if (flatbuffer.eventCount()) {
            mystats.tx_bytes += flatbuffer.produce();
          }
          flatbuffer.pulseTime(event.time);
        } else {
          // regular neutron event
          uint32_t event_time = event.time - flatbuffer.pulseTime();
          mystats.tx_bytes += flatbuffer.addEvent(event_time, event.pixel_id);
        }
      }

      XTRACE(INPUT, ALW, "Stopping processing thread.");
      return;
    }
  }
}

}
