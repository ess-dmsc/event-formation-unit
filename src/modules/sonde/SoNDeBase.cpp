// Copyright (C) 2017-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief SoNDe detector pipeline
///
//===----------------------------------------------------------------------===//

#include <sonde/SoNDeBase.h>
#include <common/EV42Serializer.h>
#include <common/monitor/HistogramSerializer.h>
#include <common/Producer.h>
#include <common/RuntimeStat.h>
#include <common/Trace.h>
#include <common/Socket.h>
#include <common/TSCTimer.h>
#include <sonde/ideas/Data.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

SONDEIDEABase::SONDEIDEABase(BaseSettings const &settings, struct SoNDeSettings & localSettings)
     : Detector("SoNDe detector using IDEAS readout", settings),
       SoNDeSettings(localSettings) {

  Stats.setPrefix(EFUSettings.GraphitePrefix, EFUSettings.GraphiteRegion);

  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off
  Stats.create("receive.packets",                 mystats.rx_packets);
  Stats.create("receive.bytes",                   mystats.rx_bytes);
  Stats.create("receive.dropped",                 mystats.fifo_push_errors);

  Stats.create("readouts.seq_errors",             mystats.rx_seq_errors);
  Stats.create("ideas.packets.triggertime",       mystats.rx_pkt_triggertime);
  Stats.create("ideas.packets.singleevent",       mystats.rx_pkt_singleevent);
  Stats.create("ideas.packets.multievent",        mystats.rx_pkt_multievent);
  Stats.create("ideas.packets.unsupported",       mystats.rx_pkt_unsupported);

  Stats.create("events.count",                    mystats.rx_events);
  Stats.create("events.geometry_errors",          mystats.rx_geometry_errors);

  Stats.create("transmit.bytes",                  mystats.tx_bytes);

  Stats.create("thread.input_idle",               mystats.rx_idle);
  Stats.create("thread.processing_idle",          mystats.processing_idle);
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
}

void SONDEIDEABase::input_thread() {
  /** Connection setup */
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(),
                         EFUSettings.DetectorPort);
  UDPReceiver sondedata(local);
  sondedata.setBufferSizes(EFUSettings.TxSocketBufferSize,
                           EFUSettings.RxSocketBufferSize);
  sondedata.checkRxBufferSizes(EFUSettings.RxSocketBufferSize);
  sondedata.printBufferSizes();
  sondedata.setRecvTimeout(0, 100000); // secs, usecs, 1/10 second

  for (;;) {
    int readSize;
    unsigned int rxBufferIndex = RxRingbuffer.getDataIndex();

    /** this is the processing step */
    RxRingbuffer.setDataLength(rxBufferIndex, 0);
    if ((readSize = sondedata.receive(RxRingbuffer.getDataBuffer(rxBufferIndex),
                                    RxRingbuffer.getMaxBufSize())) > 0) {
      mystats.rx_packets++;
      mystats.rx_bytes += readSize;
      RxRingbuffer.setDataLength(rxBufferIndex, readSize);

      if (InputFifo.push(rxBufferIndex) == false) {
        mystats.fifo_push_errors++;
      } else {
        RxRingbuffer.getNextBuffer();
      }
    } else {
      mystats.rx_idle++;
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

  Producer eventprod(EFUSettings.KafkaBroker, "skadi_detector");
  auto Produce = [&eventprod](auto DataBuffer, auto Timestamp) {
    eventprod.produce(DataBuffer, Timestamp);
  };

  EV42Serializer flatbuffer(KafkaBufferSize, "SONDE", Produce);

  constexpr uint16_t maxChannels{64};
  constexpr uint16_t maxAdc{65535};
  Hists histograms(maxChannels, maxAdc);
  HistogramSerializer histfb(histograms.needed_buffer_size(), "SONDE");
  Producer monitorprod(EFUSettings.KafkaBroker, "SKADI_monitor");

  auto ProduceHist = [&monitorprod](auto DataBuffer, auto Timestamp) {
    monitorprod.produce(DataBuffer, Timestamp);
  };

  histfb.set_callback(ProduceHist);
  unsigned int data_index;

  RuntimeStat RtStat({mystats.rx_packets, mystats.rx_events, mystats.tx_bytes});

  TSCTimer produce_timer;
  while (1) {
    if ((InputFifo.pop(data_index)) == false) {
      mystats.processing_idle++;

      if (produce_timer.timetsc() >=
          EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ) {
        mystats.tx_bytes += flatbuffer.produce();

        /// Kafka stats update - common to all detectors
        /// don't increment as producer keeps absolute count
        mystats.kafka_produce_fails = eventprod.stats.produce_fails;
        mystats.kafka_ev_errors = eventprod.stats.ev_errors;
        mystats.kafka_ev_others = eventprod.stats.ev_others;
        mystats.kafka_dr_errors = eventprod.stats.dr_errors;
        mystats.kafka_dr_noerrors = eventprod.stats.dr_noerrors;
        produce_timer.reset();

        RuntimeStatusMask =  RtStat.getRuntimeStatusMask({mystats.rx_packets, mystats.rx_events, mystats.tx_bytes});

        if (!histograms.isEmpty()) {
          XTRACE(PROCESS, DEB, "Sending histogram for %zu readouts",
                 histograms.hit_count());
          histfb.produce(histograms);
          histograms.clear();
        }
      }
      usleep(10);

    } else {

      auto len = RxRingbuffer.getDataLength(data_index);
      if (len == 0) {
        mystats.fifo_synch_errors++;
      } else {
        int events =
            ideasdata.parse_buffer(RxRingbuffer.getDataBuffer(data_index), len);

        mystats.rx_geometry_errors += ideasdata.errors;
        mystats.rx_events += ideasdata.events;
        // don't add to mystats. These internal counters are absolute
        mystats.rx_seq_errors = ideasdata.ctr_outof_sequence;
        mystats.rx_pkt_triggertime = ideasdata.counterPacketTriggerTime;
        mystats.rx_pkt_singleevent = ideasdata.counterPacketSingleEventPulseHeight;
        mystats.rx_pkt_multievent = ideasdata.counterPacketMultiEventPulseHeight;
        mystats.rx_pkt_unsupported = ideasdata.counterPacketUnsupported;

        if (events > 0) {
          for (int i = 0; i < events; i++) {
            assert(ideasdata.data[i].PixelId <= maxChannels);
            histograms.bin_x(ideasdata.data[i].PixelId, ideasdata.data[i].Adc);
            XTRACE(PROCESS, DEB, "flatbuffer.addevent[i: %d](t: %d, pix: %d)",
                   i, ideasdata.data[i].Time, ideasdata.data[i].PixelId);
            mystats.tx_bytes += flatbuffer.addEvent(ideasdata.data[i].Time,
                                                    ideasdata.data[i].PixelId);
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
