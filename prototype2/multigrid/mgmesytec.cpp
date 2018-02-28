/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief CSPEC Detector implementation
 */

#include <common/Detector.h>
#include <common/EFUArgs.h>
#include <common/FBSerializer.h>
#include <common/Producer.h>
#include <common/RingBuffer.h>
#include <common/Trace.h>
#include <cstring>
#include <efu/Parser.h>
#include <efu/Server.h>
#include <gdgem/nmx/HistSerializer.h>
#include <common/ReadoutSerializer.h>
#include <iostream>
#include <libs/include/SPSCFifo.h>
#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>
#include <memory>
#include <multigrid/mgmesytec/Data.h>
#include <queue>
#include <stdio.h>
#include <unistd.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

using namespace memory_sequential_consistent; // Lock free fifo

const int TSC_MHZ = 2900; // Not accurate, do not rely solely on this

/** ----------------------------------------------------- */
struct DetectorSettingsStruct {
  uint32_t wireThresholdLo = {0};     // accept all
  uint32_t wireThresholdHi = {65535}; // accept all
  uint32_t gridThresholdLo = {0};     // accept all
  uint32_t gridThresholdHi = {65535}; // accept all
} DetectorSettings;

void SetCLIArguments(CLI::App __attribute__((unused)) & parser) {
  parser.add_option("--wlo", DetectorSettings.wireThresholdLo,
         "minimum wire adc value for accept")->group("MGMesytec");
  parser.add_option("--whi", DetectorSettings.wireThresholdHi,
         "maximum wire adc value for accept")->group("MGMesytec");
  parser.add_option("--glo", DetectorSettings.gridThresholdLo,
         "minimum grid adc value for accept")->group("MGMesytec");
  parser.add_option("--ghi", DetectorSettings.gridThresholdHi,
         "maximum grid adc value for accept")->group("MGMesytec");
}

PopulateCLIParser PopulateParser{SetCLIArguments};

/** ----------------------------------------------------- */
const char *classname = "CSPEC Detector Mesytec readout";

class CSPEC : public Detector {
public:
  CSPEC(BaseSettings settings);
  void input_thread();

  const char *detectorname();

  /** @todo figure out the right size  of the .._max_entries  */
  static const int eth_buffer_size = 9000;
  static const int kafka_buffer_size = 1000000;

private:

  struct {
    // Input Counters
    int64_t rx_packets;
    int64_t rx_bytes;
    int64_t rx_readouts;
    int64_t rx_error_bytes;
    int64_t rx_discards;
    int64_t geometry_errors;
    int64_t rx_events;
    int64_t tx_bytes;
  } ALIGN(64) mystats;
};

CSPEC::CSPEC(BaseSettings settings) : Detector(settings) {
  Stats.setPrefix("efu.mgmesytec");

  XTRACE(INIT, ALW, "Adding stats\n");
  // clang-format off
  Stats.create("rx_packets",           mystats.rx_packets);
  Stats.create("rx_bytes",             mystats.rx_bytes);
  Stats.create("readouts",             mystats.rx_readouts);
  Stats.create("readouts_error_bytes", mystats.rx_error_bytes);
  Stats.create("readouts_discarded",   mystats.rx_discards);
  Stats.create("geometry_errors",      mystats.geometry_errors);
  Stats.create("events",               mystats.rx_events);
  Stats.create("tx_bytes",             mystats.tx_bytes);
  // clang-format on

  std::function<void()> inputFunc = [this]() { CSPEC::input_thread(); };
  Detector::AddThreadFunction(inputFunc, "input");
}

const char *CSPEC::detectorname() { return classname; }

void CSPEC::input_thread() {
  /** Connection setup */
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(), EFUSettings.DetectorPort);
  UDPServer cspecdata(local);
  cspecdata.setbuffers(EFUSettings.DetectorTxBufferSize, EFUSettings.DetectorRxBufferSize);
  cspecdata.printbuffers();
  cspecdata.settimeout(0, 100000); // One tenth of a second
  Producer producer(EFUSettings.KafkaBroker, "C-SPEC_detector");
  Producer monitorprod(EFUSettings.KafkaBroker, "C-SPEC_monitor");
  FBSerializer flatbuffer(kafka_buffer_size, producer);
  ReadoutSerializer readouts(100000, monitorprod);
  HistSerializer histfb;
  NMXHists hists;

  MesytecData dat;

  dat.setWireThreshold(DetectorSettings.wireThresholdLo, DetectorSettings.wireThresholdHi);
  dat.setGridThreshold(DetectorSettings.gridThresholdLo, DetectorSettings.gridThresholdHi);

  char buffer[9010];
  int rdsize;
  TSCTimer report_timer;
  for (;;) {
    if ((rdsize = cspecdata.receive(buffer, eth_buffer_size)) > 0) {
      mystats.rx_packets++;
      mystats.rx_bytes += rdsize;
      XTRACE(INPUT, DEB, "rdsize: %u\n", rdsize);

      auto res = dat.parse(buffer, rdsize, hists, readouts);

      if (res < 0) {
        continue;
      }

      mystats.rx_readouts += dat.readouts;
      mystats.rx_discards += dat.discards;

      int pixel = dat.getPixel();
      int time  = dat.getTime();
      XTRACE(PROCESS, DEB, "Time %d, pixel: %d\n", time, pixel);
      if (pixel != 0) {
        mystats.tx_bytes += flatbuffer.addevent(time, pixel);
        mystats.rx_events++;
      } else {
        mystats.geometry_errors++;
      }
    }

    // Checking for exit
    if (report_timer.timetsc() >= EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ) {
      mystats.tx_bytes += flatbuffer.produce();

      auto entries = readouts.getNumEntries();
      if (entries) {
        XTRACE(PROCESS, INF, "Flushing readout data for %zu readouts\n", entries);
        //readouts.produce(); // Periodically produce of readouts
      }

      if (!hists.isEmpty()) {
        XTRACE(PROCESS, INF, "Sending histogram for %zu readouts\n", hists.eventlet_count());
        char *txbuffer;
        auto len = histfb.serialize(hists, &txbuffer);
        monitorprod.produce(txbuffer, len);
        hists.clear();
      }

      report_timer.now();
    }

    // Checking for exit
    if (not runThreads) {
      XTRACE(INPUT, ALW, "Stopping processing thread.\n");
      return;
    }
  }
}

/** ----------------------------------------------------- */

class CSPECFactory : public DetectorFactory {
public:
  std::shared_ptr<Detector> create(BaseSettings settings) {
    return std::shared_ptr<Detector>(new CSPEC(settings));
  }
};

CSPECFactory Factory;
