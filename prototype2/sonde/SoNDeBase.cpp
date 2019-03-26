/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <sonde/SoNDeBase.h>
#include <common/EV42Serializer.h>
#include <common/HistSerializer.h>
#include <common/Producer.h>
#include <common/Trace.h>
#include <libs/include/Socket.h>
#include <libs/include/Timer.h>
#include <libs/include/TSCTimer.h>
#include <sonde/ideas/Data.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

SONDEIDEABase::SONDEIDEABase(BaseSettings const &settings, struct SoNDeSettings & localSettings)
     : Detector("SoNDe detector using IDEAS readout", settings),
       SoNDeSettings(localSettings) {

  Stats.setPrefix("efu.sonde", EFUSettings.GraphiteRegion);

  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off
  Stats.create("receive.packets",                 mystats.rx_packets);
  Stats.create("receive.bytes",                   mystats.rx_bytes);
  Stats.create("receive.dropped",                 mystats.fifo_push_errors);

  Stats.create("readouts.seq_errors",             mystats.rx_seq_errors);

  Stats.create("events.count",                    mystats.rx_events);
  Stats.create("events.geometry_errors",          mystats.rx_geometry_errors);

  Stats.create("transmit.bytes",                  mystats.tx_bytes);

  Stats.create("thread.uptime",                   mystats.up_time);
  Stats.create("thread.idle",                     mystats.rx_idle1);
  Stats.create("thread.fifo_synch_errors",        mystats.fifo_synch_errors);

  /// \todo Kafka stats are common to all detectors and could/should be moved
  Stats.create("kafka.produce_fails",             mystats.kafka_produce_fails);
  Stats.create("kafka.ev_errors",                 mystats.kafka_ev_errors);
  Stats.create("kafka.ev_others",                 mystats.kafka_ev_others);
  Stats.create("kafka.dr_errors",                 mystats.kafka_dr_errors);
  Stats.create("kafka.dr_others",                 mystats.kafka_dr_noerrors);


  // clang-format on
  std::function<void()> inputFunc = [this]() { SONDEIDEABase::input_thread(); };
  Detector::AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    SONDEIDEABase::processing_thread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d SONDE Rx ringbuffers of size %d",
         eth_buffer_max_entries, eth_buffer_size);
  eth_ringbuf = new RingBuffer<eth_buffer_size>(
      eth_buffer_max_entries + 11); /** \todo testing workaround */
  assert(eth_ringbuf != 0);
}

void SONDEIDEABase::input_thread() {
  /** Connection setup */
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(),
                         EFUSettings.DetectorPort);
  UDPReceiver sondedata(local);
  sondedata.setBufferSizes(0, EFUSettings.DetectorRxBufferSize);
  sondedata.printBufferSizes();
  sondedata.setRecvTimeout(0, 100000); // secs, usecs, 1/10 second

  for (;;) {
    int rdsize;
    unsigned int eth_index = eth_ringbuf->getDataIndex();

    /** this is the processing step */
    eth_ringbuf->setDataLength(eth_index, 0);
    if ((rdsize = sondedata.receive(eth_ringbuf->getDataBuffer(eth_index),
                                    eth_ringbuf->getMaxBufSize())) > 0) {
      mystats.rx_packets++;
      mystats.rx_bytes += rdsize;
      eth_ringbuf->setDataLength(eth_index, rdsize);

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

void SONDEIDEABase::processing_thread() {
  Sonde::Geometry geometry;
  Sonde::IDEASData ideasdata(&geometry, SoNDeSettings.fileprefix);

  EV42Serializer flatbuffer(kafka_buffer_size, "SONDE");
  Producer eventprod(EFUSettings.KafkaBroker, "SKADI_detector");
  flatbuffer.setProducerCallback(
    std::bind(&Producer::produce2<uint8_t>, &eventprod, std::placeholders::_1));

  constexpr uint16_t maxChannels{64};
  constexpr uint16_t maxAdc{65535};
  Hists histograms(maxChannels, maxAdc);
  HistSerializer histfb(histograms.needed_buffer_size());
  Producer monitorprod(EFUSettings.KafkaBroker, "SKADI_monitor");
  histfb.set_callback(
    std::bind(&Producer::produce2<uint8_t>, &monitorprod, std::placeholders::_1));

  unsigned int data_index;

  Timer UpTime;
  TSCTimer produce_timer;
  while (1) {
    if ((input2proc_fifo.pop(data_index)) == false) {
      mystats.rx_idle1++;

      if (produce_timer.timetsc() >=
          EFUSettings.UpdateIntervalSec * 1000000 * TscMHz) {
        mystats.tx_bytes += flatbuffer.produce();

        mystats.up_time = (int64_t)UpTime.timeus()/1000000;

        /// Kafka stats update - common to all detectors
        /// don't increment as producer keeps absolute count
        mystats.kafka_produce_fails = eventprod.stats.produce_fails;
        mystats.kafka_ev_errors = eventprod.stats.ev_errors;
        mystats.kafka_ev_others = eventprod.stats.ev_others;
        mystats.kafka_dr_errors = eventprod.stats.dr_errors;
        mystats.kafka_dr_noerrors = eventprod.stats.dr_noerrors;
        produce_timer.now();

        if (!histograms.isEmpty()) {
          XTRACE(PROCESS, DEB, "Sending histogram for %zu readouts",
                 histograms.hit_count());
          histfb.produce(histograms);
          histograms.clear();
        }
      }
      usleep(10);

    } else {

      auto len = eth_ringbuf->getDataLength(data_index);
      if (len == 0) {
        mystats.fifo_synch_errors++;
      } else {
        int events =
            ideasdata.parse_buffer(eth_ringbuf->getDataBuffer(data_index), len);

        mystats.rx_geometry_errors += ideasdata.errors;
        mystats.rx_events += ideasdata.events;
        mystats.rx_seq_errors = ideasdata.ctr_outof_sequence;

        if (events > 0) {
          for (int i = 0; i < events; i++) {
            assert(ideasdata.data[i].pixel_id < maxChannels);
            histograms.bin_x(ideasdata.data[i].pixel_id, 1000); /// \todo adc not available
            XTRACE(PROCESS, DEB, "flatbuffer.addevent[i: %d](t: %d, pix: %d)",
                   i, ideasdata.data[i].time, ideasdata.data[i].pixel_id);
            mystats.tx_bytes += flatbuffer.addEvent(ideasdata.data[i].time,
                                                    ideasdata.data[i].pixel_id);
          }
        }
      }
    }
    if (not runThreads) {
      XTRACE(INPUT, ALW, "Stopping input thread.");
      return;
    }
  }
}
