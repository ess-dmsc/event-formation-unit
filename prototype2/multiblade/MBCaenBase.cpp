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
#include <common/DataSave.h>
#include <unistd.h>

#include <libs/include/SPSCFifo.h>
#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>

#include <mbcaen/DataParser.h>

#include <mbcommon/MultiBladeEventBuilder.h>

#include <logical_geometry/ESSGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

using namespace memory_sequential_consistent; // Lock free fifo

const char *classname = "Multiblade detector with CAEN readout";

const int TSC_MHZ = 2900; // MJC's workstation - not reliable


MBCAENBase::MBCAENBase(BaseSettings const &settings, struct MBCAENSettings & LocalMBCAENSettings)
    : Detector("MBCAEN", settings), MBCAENSettings(LocalMBCAENSettings) {
  Stats.setPrefix("efu.mbcaen");

  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off
  Stats.create("input.rx_packets",                mystats.rx_packets);
  Stats.create("input.rx_bytes",                  mystats.rx_bytes);
  Stats.create("input.fifo1_push_errors",         mystats.fifo1_push_errors);
  Stats.create("processing.rx_readouts",          mystats.rx_readouts);
  Stats.create("processing.rx_idle1",             mystats.rx_idle1);
  Stats.create("processing.tx_bytes",             mystats.tx_bytes);
  Stats.create("processing.rx_events",            mystats.rx_events);
  Stats.create("processing.rx_geometry_errors",   mystats.geometry_errors);
  Stats.create("processing.fifo_seq_errors",      mystats.fifo_seq_errors);
  /// \todo below stats are common to all detectors and could/should be moved
  Stats.create("kafka_produce_fails", mystats.kafka_produce_fails);
  Stats.create("kafka_ev_errors", mystats.kafka_ev_errors);
  Stats.create("kafka_ev_others", mystats.kafka_ev_others);
  Stats.create("kafka_dr_errors", mystats.kafka_dr_errors);
  Stats.create("kafka_dr_others", mystats.kafka_dr_noerrors);
  // clang-format on

  std::function<void()> inputFunc = [this]() { MBCAENBase::input_thread(); };
  Detector::AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    MBCAENBase::processing_thread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d Multiblade Rx ringbuffers of size %d",
         eth_buffer_max_entries, eth_buffer_size);
  /// \todo the number 11 is a workaround
  eth_ringbuf = new RingBuffer<eth_buffer_size>(eth_buffer_max_entries + 11);
  assert(eth_ringbuf != 0);

  mb_opts = MBConfig(MBCAENSettings.ConfigFile);
  assert(mb_opts.getDetector() != nullptr);
}

void MBCAENBase::input_thread() {
  /** Connection setup */
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(),
                         EFUSettings.DetectorPort);
  UDPReceiver mbdata(local);
  // mbdata.buflen(opts->buflen);
  mbdata.setBufferSizes(0, EFUSettings.DetectorRxBufferSize);
  mbdata.printBufferSizes();
  mbdata.setRecvTimeout(0, 100000); /// secs, usecs 1/10s

  int rdsize;
  for (;;) {
    unsigned int eth_index = eth_ringbuf->getDataIndex();

    /** this is the processing step */
    eth_ringbuf->setDataLength(eth_index, 0);
    if ((rdsize = mbdata.receive(eth_ringbuf->getDataBuffer(eth_index),
                                 eth_ringbuf->getMaxBufSize())) > 0) {
      eth_ringbuf->setDataLength(eth_index, rdsize);
      XTRACE(PROCESS, DEB, "Received an udp packet of length %d bytes",
             rdsize);
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

void MBCAENBase::processing_thread() {
  const uint32_t ncass = 6;
  uint8_t nwires = 32;
  uint8_t nstrips = 32;

  std::shared_ptr<DataSave> mbdatasave;
  bool dumptofile = !MBCAENSettings.FilePrefix.empty();
  if (dumptofile)
  {
    mbdatasave = std::make_shared<DataSave>(MBCAENSettings.FilePrefix + "_multiblade_", 100000000);
    mbdatasave->tofile("# time, digitizer, channel, adc\n");
  }

  ESSGeometry essgeom(nstrips, ncass * nwires, 1, 1);

  EV42Serializer flatbuffer(kafka_buffer_size, "multiblade");
  Producer eventprod(EFUSettings.KafkaBroker, "MB_detector");
  flatbuffer.setProducerCallback(
      std::bind(&Producer::produce2<uint8_t>, &eventprod, std::placeholders::_1));

  Hists histograms(std::max(nwires, nstrips), 65535);
  Producer monitorprod(EFUSettings.KafkaBroker, "MB_monitor");
  HistSerializer histfb(histograms.needed_buffer_size());
  histfb.set_callback(
      std::bind(&Producer::produce2<uint8_t>, &monitorprod, std::placeholders::_1));

  MultiBladeEventBuilder builder[ncass];
  for (uint32_t i = 0; i < ncass; i++) {
    builder[i].setNumberOfWireChannels(nwires);
    builder[i].setNumberOfStripChannels(nstrips);
  }

  DataParser mbdata;
  auto digitisers = mb_opts.getDigitisers();
  MB16Detector mb16(digitisers);

  unsigned int data_index;
  TSCTimer produce_timer;
  while (1) {
    if ((input2proc_fifo.pop(data_index)) == false) {
      // There is NO data in the FIFO - do stop checks and sleep a little
      mystats.rx_idle1++;
      // Checking for exit
      if (produce_timer.timetsc() >=
          EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ) {

        mystats.tx_bytes += flatbuffer.produce();

        if (!histograms.isEmpty()) {
          XTRACE(PROCESS, INF, "Sending histogram for %zu readouts", histograms.hit_count());
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

        if (not runThreads) {
          XTRACE(INPUT, ALW, "Stopping processing thread.");
          return;
        }

        produce_timer.now();
      }
      usleep(10);

    } else { // There is data in the FIFO - do processing
      auto datalen = eth_ringbuf->getDataLength(data_index);
      if (datalen == 0) {
        mystats.fifo_seq_errors++;
      } else {
        auto dataptr = eth_ringbuf->getDataBuffer(data_index);
        if (mbdata.parse(dataptr, datalen) < 0) {
          mystats.rx_error_bytes += mbdata.Stats.error_bytes;
          continue;
        }

        mystats.rx_readouts += mbdata.MBHeader->numElements;

        auto digitizerId = mbdata.MBHeader->digitizerID;
        XTRACE(DATA, DEB, "Received %d readouts from digitizer %d\n", mbdata.MBHeader->numElements, digitizerId);
        for (uint i = 0; i < mbdata.MBHeader->numElements; i++) {

          auto dp = mbdata.MBData[i];
          // XTRACE(DATA, DEB, "digitizer: %d, time: %d, channel: %d, adc: %d\n",
          //       digitizerId, dp.localTime, dp.channel, dp.adcValue);

          /// \todo why can't I use mb_opts.detector->cassette()
          auto cassette = mb16.cassette(digitizerId);

          if (cassette < 0) {
            XTRACE(DATA, WAR, "Invalid digitizerId: %d\n", digitizerId);

            break;
          }

          if (dumptofile) {
            mbdatasave->tofile("%d,%d,%d,%d\n", dp.localTime, digitizerId, dp.channel, dp.adcValue);
          }

          if (dp.channel >= 32) {
            histograms.binstrips(dp.channel, dp.adcValue, 0, 0);
          } else {
            histograms.binstrips(0, 0, dp.channel, dp.adcValue);
          }

          if (builder[cassette].addDataPoint(dp.channel, dp.adcValue, dp.localTime)) {

            auto xcoord = builder[cassette].getStripPosition() - 32; // pos 32 - 63
            auto ycoord = cassette * nwires +
                          builder[cassette].getWirePosition(); // pos 0 - 31

            uint32_t pixel_id = essgeom.pixel2D(xcoord, ycoord);

            XTRACE(PROCESS, DEB,
                   "digi: %d, wire: %d, strip: %d, x: %d, y:%d, pixel_id: %d\n",
                   digitizerId, (int)xcoord, (int)ycoord,
                   (int)builder[cassette].getWirePosition(),
                   (int)builder[cassette].getStripPosition(), pixel_id);

            if (pixel_id == 0) {
              mystats.geometry_errors++;
            } else {
              mystats.tx_bytes += flatbuffer.addEvent(
                  builder[cassette].getTimeStamp(), pixel_id);
              mystats.rx_events++;
            }
          }
        }
      }
    }
  }
}
