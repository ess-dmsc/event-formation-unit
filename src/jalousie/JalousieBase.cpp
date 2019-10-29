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
#include <common/Producer.h>
#include <common/RingBuffer.h>
#include <common/TimeString.h>
#include <common/TestImageUdder.h>

#include <common/SPSCFifo.h>
#include <common/Socket.h>
#include <common/TSCTimer.h>
#include <common/Timer.h>

#include <common/Trace.h>
#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

#include <common/Log.h>
#include "JalousieBase.h"
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
  Stats.create("receive.packets", Counters.RxPackets);
  Stats.create("receive.bytes", Counters.RxBytes);
  Stats.create("receive.dropped", Counters.FifoPushErrors);

  Stats.create("readouts.count", Counters.ReadoutCount);
  Stats.create("readouts.BadModuleId", Counters.BadModuleId);
  Stats.create("readouts.ChopperPulses", Counters.ChopperPulses);

  Stats.create("events.count", Counters.Events);
  Stats.create("events.MappingErrors", Counters.MappingErrors);
  Stats.create("events.GeometryErrors", Counters.GeometryErrors);
  Stats.create("events.TimingErrors", Counters.TimingErrors);
  Stats.create("transmit.bytes", Counters.TxBytes);

  Stats.create("thread.seq_errors", Counters.FifoSeqErrors);
  Stats.create("thread.ProcessingIdle", Counters.ProcessingIdle);

  /// \todo below stats are common to all detectors and could/should be moved
  Stats.create("kafka.produce_fails", Counters.kafka_produce_fails);
  Stats.create("kafka.ev_errors", Counters.kafka_ev_errors);
  Stats.create("kafka.ev_others", Counters.kafka_ev_others);
  Stats.create("kafka.dr_errors", Counters.kafka_dr_errors);
  Stats.create("kafka.dr_others", Counters.kafka_dr_noerrors);
  // clang-format on

  std::function<void()> inputFunc = [this]() { JalousieBase::inputThread(); };
  Detector::AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    JalousieBase::processingThread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d Jalousie Rx ringbuffers of size %d",
         eth_buffer_max_entries, eth_buffer_size);
  /// \todo the number 11 is a workaround
  eth_ringbuf = new RingBuffer<eth_buffer_size>(eth_buffer_max_entries + 11);
  assert(eth_ringbuf != 0);
}

void JalousieBase::inputThread() {
  /** Connection setup */
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(),
                         EFUSettings.DetectorPort);
  UDPReceiver receiver(local);
  // receiver.buflen(opts->buflen);
  receiver.setBufferSizes(0, EFUSettings.DetectorRxBufferSize);
  receiver.checkRxBufferSizes(EFUSettings.DetectorRxBufferSize);
  receiver.printBufferSizes();
  receiver.setRecvTimeout(0, 100000); /// secs, usecs 1/10s

  while (runThreads) {
    int ReadSize;
    unsigned int eth_index = eth_ringbuf->getDataIndex();

    /** this is the processing step */
    eth_ringbuf->setDataLength(eth_index, 0);
    if ((ReadSize = receiver.receive(eth_ringbuf->getDataBuffer(eth_index),
                                   eth_ringbuf->getMaxBufSize())) > 0) {
      eth_ringbuf->setDataLength(eth_index, ReadSize);
     XTRACE(INPUT, DEB, "Received an udp packet of length %d bytes",
            ReadSize);
      Counters.RxPackets++;
      Counters.RxBytes += ReadSize;

      if (input2proc_fifo.push(eth_index) == false) {
        Counters.FifoPushErrors++;
      } else {
        eth_ringbuf->getNextBuffer();
      }
    }
  }
  XTRACE(INPUT, ALW, "Stopping input thread.");
  return;
}

void JalousieBase::convert_and_enqueue_event(const Readout &readout) {

  /// Must have valid board mapping to proceed
  constexpr size_t invalid_board_mapping{std::numeric_limits<size_t>::max()};
  size_t module = config.board_mappings[readout.board];
  if (module == invalid_board_mapping) {
    Counters.BadModuleId++;
    return;
  }

  if (readout.sub_id == Readout::chopper_sub_id) {
    /// If this is a pulse, then we queue this with pixel_id=0
    Counters.ChopperPulses++;
    config.merger.insert(module, {readout.time, 0});
  } else {
    uint32_t pixel_id{0};
    if (config.SUMO_mappings.size() > module) {
      /// SUMO mappings appear to be available, let's use this for pixel id
      SumoCoordinates c = config.SUMO_mappings[module].map(readout.anode, readout.cathode);
      if (!c.is_valid()) {
        Counters.MappingErrors++;
        return;
      }
      pixel_id = config.geometry.pixelMP3D(c.wire_layer, c.wire, c.strip, module);
    } else {
      /// SUMO mappings are not available, naive map entire module onto plane
      pixel_id = config.geometry.pixelMP2D(readout.anode, readout.cathode, module);
    }

    /// In this case, pixel_id cannot be 0, we must reject before queueing
    if (pixel_id == 0) {
      Counters.GeometryErrors++;
    } else {
      config.merger.insert(module, {readout.time, pixel_id});
      Counters.Events++;
    }
  }
}

void JalousieBase::process_one_queued_event(EV42Serializer &serializer) {
  auto event = config.merger.pop_earliest();
  if (previous_time > event.time)
    Counters.TimingErrors++;
  previous_time = event.time;

  if (event.pixel_id == 0) {
    /// chopper pulse
    if (serializer.eventCount()) {
      /// Flush any events if incrementing pulse
      Counters.TxBytes += serializer.produce();
    }
    serializer.pulseTime(event.time);
  } else {     /// regular neutron event
    /// rebaste time in terms of most recent pulse
    uint32_t event_time = event.time - serializer.pulseTime();
    Counters.TxBytes += serializer.addEvent(event_time, event.pixel_id);
  }
}

void JalousieBase::force_produce_and_update_kafka_stats(
    EV42Serializer &serializer,
    Producer &producer) {
  Counters.TxBytes += serializer.produce();
  /// Kafka stats update - common to all detectors
  /// don't increment as producer keeps absolute count
  Counters.kafka_produce_fails = producer.stats.produce_fails;
  Counters.kafka_ev_errors = producer.stats.ev_errors;
  Counters.kafka_ev_others = producer.stats.ev_others;
  Counters.kafka_dr_errors = producer.stats.dr_errors;
  Counters.kafka_dr_noerrors = producer.stats.dr_noerrors;
}

void JalousieBase::processingThread() {
  LOG(INIT, Sev::Info, "Jalousie Config file: {}", ModuleSettings.ConfigFile);
  config = Config(ModuleSettings.ConfigFile);
  LOG(INIT, Sev::Info, "Jalousie Config\n{}", config.debug());

  Producer EventProducer(EFUSettings.KafkaBroker, "DREAM_detector");

  auto Produce = [&EventProducer](auto DataBuffer, auto Timestamp) {
    EventProducer.produce(DataBuffer, Timestamp);
  };

  EV42Serializer ev42Serializer(kafka_buffer_size, "jalo", Produce);

  unsigned int data_index;
  TSCTimer produce_timer;
  while (true) {
    if (input2proc_fifo.pop(data_index)) { // There is data in the FIFO - do processing
      auto datalen = eth_ringbuf->getDataLength(data_index);
      if (datalen == 0) {
        Counters.FifoSeqErrors++;
        continue;
      }

      auto dataptr = eth_ringbuf->getDataBuffer(data_index);
      assert(datalen % sizeof(Jalousie::Readout) == 0);

      /// Parse and convert readouts to events
      for (size_t i = 0; i < datalen / sizeof(Jalousie::Readout); i++) {
        auto readout = (Jalousie::Readout *) dataptr;
       // printf("time %lu, board: %u, sub_id: %u, anode: %u, cathode: %u\n",
       //    readout->time, readout->board, readout->sub_id, readout->anode, readout->cathode);
        dataptr += sizeof(Jalousie::Readout);
        Counters.ReadoutCount++;
        convert_and_enqueue_event(*readout);
      }

      /// Send out events that pass max latency check
      config.merger.sort();
      while (config.merger.ready()) {
        process_one_queued_event(ev42Serializer);
      }

    } else {
      /// There is NO data in the FIFO - do stop checks and sleep a little
      Counters.ProcessingIdle++;
      usleep(10);
    }

    /// Periodic producing regardless of rates
    if (produce_timer.timetsc() >=
        EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ) {
      force_produce_and_update_kafka_stats(ev42Serializer, EventProducer);
      produce_timer.now();
    }

    if (not runThreads) {
      /// Send out all events, regardless of max latency criterion
      config.merger.sort();
      while (!config.merger.empty()) {
        process_one_queued_event(ev42Serializer);
      }
      force_produce_and_update_kafka_stats(ev42Serializer, EventProducer);
      XTRACE(INPUT, ALW, "Stopping processing thread.");
      return;
    }
  }
}

}
