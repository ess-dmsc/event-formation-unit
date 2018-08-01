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
#include <common/EV42Serializer.h>
#include <common/Producer.h>
#include <common/RingBuffer.h>
#include <common/TimeString.h>
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

#include <multigrid/mgmesytec/MgEfuMaximum.h>
#include <multigrid/mgmesytec/MgEfuCenterMass.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

const int TSC_MHZ = 2900; // Not accurate, do not rely solely on this

/** ----------------------------------------------------- */
struct DetectorSettingsStruct {
  bool monitor{false};
  std::string fileprefix{""};

  bool swap_wires{false};
  uint32_t module{0}; // flipped z
  bool spoof_high_time{false};

  std::string reduction_strategy;

  uint32_t wireThresholdLo{0};     // accept all
  uint32_t wireThresholdHi{65535}; // accept all
  uint32_t gridThresholdLo{0};     // accept all
  uint32_t gridThresholdHi{65535}; // accept all

  std::string debug() const;
} DetectorSettings;

void SetCLIArguments(CLI::App & parser) {
  parser.add_flag("--monitor", DetectorSettings.monitor,
                  "stream monitor data")->group("MGMesytec")->configurable(true)->set_default_val("true");
  parser.add_option("--dumptofile", DetectorSettings.fileprefix,
                    "dump to specified file")->group("MGMesytec")->configurable(true);

  parser.add_flag("--swap_wires", DetectorSettings.swap_wires,
                  "swap odd and even wires")->group("MGMesytec")->configurable(true)->set_default_val("true");
  parser.add_option("--module", DetectorSettings.module,
         "select module for reversing z coordinates")->group("MGMesytec")->
         configurable(true)->set_default_val("1");

  parser.add_flag("--spoof_high", DetectorSettings.spoof_high_time,
                  "spoof high time")->group("MGMesytec")->configurable(true);

  parser.add_option("--strategy", DetectorSettings.reduction_strategy,
                    "reduction strategy (maximum, center-mass)")->group("MGMesytec")->configurable(true)->set_default_val("maximum");

  parser.add_option("--wlo", DetectorSettings.wireThresholdLo,
                    "minimum wire adc value for accept")->group("MGMesytec")->configurable(true)->set_default_val("0");
  parser.add_option("--whi", DetectorSettings.wireThresholdHi,
                    "maximum wire adc value for accept")->group("MGMesytec")->configurable(true)->set_default_val("65535");
  parser.add_option("--glo", DetectorSettings.gridThresholdLo,
                    "minimum grid adc value for accept")->group("MGMesytec")->configurable(true)->set_default_val("0");
  parser.add_option("--ghi", DetectorSettings.gridThresholdHi,
                    "maximum grid adc value for accept")->group("MGMesytec")->configurable(true)->set_default_val("65535");
}

std::string DetectorSettingsStruct::debug() const {
  std::stringstream ss;
  ss << "  ===============================================\n";
  ss << "  ========       multigrid mesytec       ========\n";
  ss << "  ===============================================\n";
  ss << "  Stream monitor data = " << (monitor ? "YES" : "no") << "\n";
  if (!fileprefix.empty())
    ss << "  Dump h5 data in path = " << fileprefix << "\n";
  ss << "\n";
  ss << "  Spoof high time = " << (spoof_high_time ? "YES" : "no") << "\n";
  ss << "  Swap odd/even wires = " << (swap_wires ? "YES" : "no") << "\n";
  ss << "  Flip z coordinates on bus# = " << module << "\n";
  ss << "  Event reduction strategy: " << reduction_strategy << "\n";
  if (reduction_strategy == "maximum") {
    ss << "    Thresholds:\n";
    ss << "     wire min = " << wireThresholdLo << "\n";
    ss << "     wire max = " << wireThresholdHi << "\n";
    ss << "     grid min = " << gridThresholdLo << "\n";
    ss << "     grid max = " << gridThresholdHi << "\n";
  }
  return ss.str();
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
  static constexpr int eth_buffer_size = 9000;          /// used for experimentation
  const int kafka_buffer_size = 1000000;     /// -||-
  const int readout_entries = 100000;        /// number of raw readout entries
  const int one_tenth_second_usecs = 100000; ///

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

  XTRACE(INIT, ALW, "Detector Settings:\n%s", DetectorSettings.debug().c_str());
}

const char *CSPEC::detectorname() { return classname; }

struct Monitor
{
  std::shared_ptr<Hists> hists;
  std::shared_ptr<ReadoutSerializer> readouts;

  void init(std::string broker, size_t max_readouts) {
    readouts = std::make_shared<ReadoutSerializer>(max_readouts);
    hists = std::make_shared<Hists>(std::numeric_limits<uint16_t>::max(),
                                    std::numeric_limits<uint16_t>::max());
    histfb = std::make_shared<HistSerializer>(hists->needed_buffer_size());

    producer = std::make_shared<Producer>(broker, "C-SPEC_monitor");
    readouts->set_callback(std::bind(&Producer::produce2, producer.get(), std::placeholders::_1));
    histfb->set_callback(std::bind(&Producer::produce2, producer.get(), std::placeholders::_1));
    enabled_ = true;
  }

  void close() {
    enabled_ = false;
    hists.reset();
    readouts.reset();
    histfb.reset();
    producer.reset();
  }

  void produce() {
    if (!enabled_)
      return;

    if (!hists->isEmpty()) {
      XTRACE(PROCESS, INF, "Sending histogram for %zu readouts\n", hists->hit_count());
      histfb->produce(*hists);
      hists->clear();
    }

    if (readouts->getNumEntries()) {
      XTRACE(PROCESS, INF, "Flushing readout data for %zu readouts\n", readouts->getNumEntries());
      readouts->produce();
    }
  }

  bool enabled() const {
    return enabled_;
  }

  Monitor() = default;
  ~Monitor() { close(); }

private:
  bool enabled_ {false};

  std::shared_ptr<Producer> producer;
  std::shared_ptr<HistSerializer> histfb;
};

void CSPEC::mainThread() {
  /** Connection setup */
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(), EFUSettings.DetectorPort); //Change name or add more comments
  UDPReceiver cspecdata(local);
  cspecdata.setBufferSizes(EFUSettings.DetectorTxBufferSize, EFUSettings.DetectorRxBufferSize);
  cspecdata.printBufferSizes();
  cspecdata.setRecvTimeout(0, one_tenth_second_usecs); /// secs, usecs

  EV42Serializer flatbuffer(kafka_buffer_size, "multigrid");
  Producer EventProducer(EFUSettings.KafkaBroker, "C-SPEC_detector");
  flatbuffer.set_callback(std::bind(&Producer::produce2, &EventProducer, std::placeholders::_1));

  Monitor monitor;
  if (DetectorSettings.monitor)
    monitor.init(EFUSettings.KafkaBroker, readout_entries);

  auto mg_mappings = std::make_shared<MgSeqGeometry>();
  mg_mappings->select_module(DetectorSettings.module);
  mg_mappings->swap_on(DetectorSettings.swap_wires);
  std::shared_ptr<MgEFU> mg_efu;
  if (DetectorSettings.reduction_strategy == "center-mass") {
    mg_efu = std::make_shared<MgEfuCenterMass>();
  } else {
    auto mg_efum = std::make_shared<MgEfuMaximum>();
    mg_efum->setWireThreshold(DetectorSettings.wireThresholdLo, DetectorSettings.wireThresholdHi);
    mg_efum->setGridThreshold(DetectorSettings.gridThresholdLo, DetectorSettings.gridThresholdHi);
    mg_efu = mg_efum;
  }
  mg_efu->mappings = mg_mappings;
  mg_efu->hists = monitor.hists;

  std::shared_ptr<MGHitFile> dumpfile;
  if (!DetectorSettings.fileprefix.empty())
  {
    dumpfile = std::make_shared<MGHitFile>();
    dumpfile->open_rw(DetectorSettings.fileprefix + "mgmesytec_" + timeString() + ".h5");
  }
  MesytecData mesytecdata(mg_efu, monitor.readouts,
      DetectorSettings.spoof_high_time, dumpfile);

  char buffer[eth_buffer_size];
  int ReadSize;
  TSCTimer report_timer;
  for (;;) {
    if ((ReadSize = cspecdata.receive(buffer, eth_buffer_size)) > 0) {
      mystats.rx_packets++;
      mystats.rx_bytes += ReadSize;
      XTRACE(INPUT, DEB, "read size: %u\n", ReadSize);

      auto res = mesytecdata.parse(buffer, ReadSize, flatbuffer);
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
      monitor.produce();
      report_timer.now();
    }

    // Checking for exit
    if (not runThreads) {
      // flush anything that remains
      mystats.tx_bytes += flatbuffer.produce();
      monitor.produce();
      XTRACE(INPUT, ALW, "Stopping processing thread.\n");
      return;
    }
  }
}

/** ----------------------------------------------------- */

DetectorFactory<CSPEC> Factory;
