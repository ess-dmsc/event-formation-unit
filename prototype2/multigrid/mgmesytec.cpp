/** Copyright (C) 2016-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Processing pipeline for CSPEC instrument (Multi-Grid detector using
/// Mesytec readout)
///
//===----------------------------------------------------------------------===//

#include <common/Detector.h>
#include <common/EFUArgs.h>
#include <common/FBSerializer.h>
#include <common/Producer.h>
#include <common/RingBuffer.h>
#include <common/Trace.h>
#include <cstring>
#include <efu/Parser.h>
#include <efu/Server.h>
#include <common/HistSerializer.h>
#include <common/ReadoutSerializer.h>
#include <iostream>
#include <libs/include/SPSCFifo.h>
#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>
#include <memory>
#include <multigrid/mgmesytec/DataParser.h>
#include <queue>
#include <stdio.h>
#include <unistd.h>

#include <multigrid/mgmesytec/MgSeqGeometry.h>
#include <multigrid/mgmesytec/MG24Geometry.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

const int TSC_MHZ = 2900; // Not accurate, do not rely solely on this

/** ----------------------------------------------------- */
struct DetectorSettingsStruct {
  uint32_t wireThresholdLo{0};     // accept all
  uint32_t wireThresholdHi{65535}; // accept all
  uint32_t gridThresholdLo{0};     // accept all
  uint32_t gridThresholdHi{65535}; // accept all
  uint32_t module{0}; // 0 defaults to 16 wires in z, 1
  std::string fileprefix{""};
} DetectorSettings;

void SetCLIArguments(CLI::App & parser) {
  parser.add_option("--wlo", DetectorSettings.wireThresholdLo,
         "minimum wire adc value for accept")->group("MGMesytec");
  parser.add_option("--whi", DetectorSettings.wireThresholdHi,
         "maximum wire adc value for accept")->group("MGMesytec");
  parser.add_option("--glo", DetectorSettings.gridThresholdLo,
         "minimum grid adc value for accept")->group("MGMesytec");
  parser.add_option("--ghi", DetectorSettings.gridThresholdHi,
         "maximum grid adc value for accept")->group("MGMesytec");
  parser.add_option("--module", DetectorSettings.module,
         "select module for correct wire swapping (0==16z, 1==20z)")->group("MGMesytec");
  parser.add_option("--dumptofile", DetectorSettings.fileprefix,
         "dump to specified file")->group("MGMesytec");
}

PopulateCLIParser PopulateParser{SetCLIArguments};

/** ----------------------------------------------------- */
const char *classname = "CSPEC Detector Mesytec readout";

///
class CSPEC : public Detector {
public:
  CSPEC(BaseSettings settings);
  void mainThread();

  const char *detectorname();

  /// Some hardcoded constants
  static const int eth_buffer_size = 9000;          /// used for experimentation
  static const int kafka_buffer_size = 1000000;     /// -||-
  static const int readout_entries = 100000;        /// number of raw readout entries
  static const int one_tenth_second_usecs = 100000; ///

private:

  struct {
    // Input Counters
    int64_t rx_packets;
    int64_t rx_bytes;
    int64_t triggers;
    int64_t badtriggers;
    int64_t rx_readouts;
    int64_t parse_errors;
    int64_t rx_discards;
    int64_t geometry_errors;
    int64_t rx_events;
    int64_t tx_bytes;
  } ALIGN(64) mystats;
};

CSPEC::CSPEC(BaseSettings settings) : Detector("CSPEC", settings) {
  Stats.setPrefix("efu.mgmesytec");

  XTRACE(INIT, ALW, "Adding stats\n");
  // clang-format off
  Stats.create("rx_packets",            mystats.rx_packets);
  Stats.create("rx_bytes",              mystats.rx_bytes);
  Stats.create("readouts",              mystats.rx_readouts);
  Stats.create("triggers",              mystats.triggers);
  Stats.create("badtriggers",           mystats.badtriggers);
  Stats.create("readouts_parse_errors", mystats.parse_errors);
  Stats.create("readouts_discarded",    mystats.rx_discards);
  Stats.create("geometry_errors",       mystats.geometry_errors);
  Stats.create("events",                mystats.rx_events);
  Stats.create("tx_bytes",              mystats.tx_bytes);
  // clang-format on

  std::function<void()> inputFunc = [this]() { CSPEC::mainThread(); };
  Detector::AddThreadFunction(inputFunc, "main");
}

const char *CSPEC::detectorname() { return classname; }

// Maybe change the name of the function to e.g. main_thread
void CSPEC::mainThread() {
  /** Connection setup */
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(), EFUSettings.DetectorPort); //Change name or add more comments
  UDPReceiver cspecdata(local);
  cspecdata.setBufferSizes(EFUSettings.DetectorTxBufferSize, EFUSettings.DetectorRxBufferSize);
  cspecdata.printBufferSizes();
  cspecdata.setRecvTimeout(0, one_tenth_second_usecs); /// secs, usecs
  Producer EventProducer(EFUSettings.KafkaBroker, "C-SPEC_detector");
  Producer monitorprod(EFUSettings.KafkaBroker, "C-SPEC_monitor");
  FBSerializer flatbuffer(kafka_buffer_size, EventProducer);

  ReadoutSerializer readouts(readout_entries, monitorprod);
  HistSerializer histfb;
  NMXHists hists;

  auto mg_mappings = std::make_shared<MgSeqGeometry>();
  mg_mappings->select_module(DetectorSettings.module);
  mg_mappings->swap_on(true);
  MesytecData mesytecdata(mg_mappings, DetectorSettings.fileprefix);

  mesytecdata.setWireThreshold(DetectorSettings.wireThresholdLo, DetectorSettings.wireThresholdHi);
  mesytecdata.setGridThreshold(DetectorSettings.gridThresholdLo, DetectorSettings.gridThresholdHi);

  char buffer[eth_buffer_size];
  int ReadSize;
  TSCTimer report_timer;
  for (;;) {
    if ((ReadSize = cspecdata.receive(buffer, eth_buffer_size)) > 0) {
      mystats.rx_packets++;
      mystats.rx_bytes += ReadSize;
      XTRACE(INPUT, DEB, "read size: %u\n", ReadSize);

      auto res = mesytecdata.parse(buffer, ReadSize, hists, flatbuffer, readouts);
      if (res != MesytecData::error::OK) {
        mystats.parse_errors++;
      }

      mystats.rx_readouts += mesytecdata.stats.readouts;
      mystats.rx_discards += mesytecdata.stats.discards;
      mystats.triggers += mesytecdata.stats.triggers;
      mystats.badtriggers += mesytecdata.stats.badtriggers;
      mystats.geometry_errors+= mesytecdata.stats.geometry_errors;
      mystats.tx_bytes += mesytecdata.stats.tx_bytes;
      mystats.rx_events += mesytecdata.stats.events;
    }

    // Force periodic flushing
    if (report_timer.timetsc() >= EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ) {
      mystats.tx_bytes += flatbuffer.produce();

      if (!hists.isEmpty()) {
        XTRACE(PROCESS, INF, "Sending histogram for %zu readouts\n", hists.hit_count());
        char *txbuffer;
        auto len = histfb.serialize(hists, &txbuffer);
        monitorprod.produce(txbuffer, len);
        hists.clear();
      }

      report_timer.now();
    }

    // Checking for exit
    if (not runThreads) {
      mystats.tx_bytes += flatbuffer.produce();
      auto entries = readouts.getNumEntries();
      if (entries > 0) {
        XTRACE(PROCESS, INF, "Flushing readout data for %zu readouts\n", entries);
        //readouts.produce(); // Periodically produce of readouts
      }

      XTRACE(INPUT, ALW, "Stopping processing thread.\n");
      return;
    }
  }
}

/** ----------------------------------------------------- */

DetectorFactory<CSPEC> Factory;
