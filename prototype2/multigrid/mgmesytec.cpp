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
#include <multigrid/MgConfig.h>
#include <queue>
#include <stdio.h>
#include <unistd.h>

#include <multigrid/mgmesytec/MgSeqGeometry.h>
//#include <multigrid/mgmesytec/MG24Geometry.h>

#include <multigrid/mgmesytec/MgEfuMaximum.h>
#include <multigrid/mgmesytec/MgEfuCenterMass.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

const int TSC_MHZ = 2900; // Not accurate, do not rely solely on this

/** ----------------------------------------------------- */
struct DetectorSettingsStruct {
  std::string ConfigFile;
  bool monitor{false};
  std::string fileprefix{""};
} DetectorSettings;

void SetCLIArguments(CLI::App & parser) {
  parser.add_option("-f,--file", DetectorSettings.ConfigFile,
                    "NMX (gdgem) specific config file")->group("MGMesytec")->
                    required()->configurable(true);
  parser.add_flag("--monitor", DetectorSettings.monitor,
                  "stream monitor data")->group("MGMesytec")->configurable(true)->set_default_val("true");
  parser.add_option("--dumptofile", DetectorSettings.fileprefix,
                    "dump to specified file")->group("MGMesytec")->configurable(true);
}

PopulateCLIParser PopulateParser{SetCLIArguments};

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

  void init_config();

  MgConfig mg_config;
  Monitor monitor;
  std::shared_ptr<MesytecData> mesytecdata;

};

CSPEC::CSPEC(BaseSettings settings) : Detector("CSPEC", settings) {
  Stats.setPrefix("efu.mgmesytec");

  XTRACE(INIT, ALW, "Adding stats");
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

  XTRACE(INIT, ALW, "Stream monitor data = %s\n",
      (DetectorSettings.monitor ? "YES" : "no"));
  if (!DetectorSettings.fileprefix.empty())
    XTRACE(INIT, ALW, "Dump h5 data in path: %s\n",
           DetectorSettings.fileprefix.c_str());
}

const char *CSPEC::detectorname() { return classname; }



void CSPEC::init_config()
{
  XTRACE(PROCESS, ALW, "MG Config file: %s\n", DetectorSettings.ConfigFile.c_str());
  mg_config = MgConfig(DetectorSettings.ConfigFile);
  XTRACE(PROCESS, ALW, "Multigrid Config\n%s\n", mg_config.debug().c_str());

  if (DetectorSettings.monitor)
    monitor.init(EFUSettings.KafkaBroker, readout_entries);

  std::shared_ptr<MgEFU> mg_efu;
  if (mg_config.reduction_strategy == "center-mass") {
    mg_efu = std::make_shared<MgEfuCenterMass>();
  } else {
    auto mg_efum = std::make_shared<MgEfuMaximum>();
    mg_efum->setWireThreshold(mg_config.wireThresholdLo, mg_config.wireThresholdHi);
    mg_efum->setGridThreshold(mg_config.gridThresholdLo, mg_config.gridThresholdHi);
    mg_efu = mg_efum;
  }
  mg_efu->mappings = mg_config.mappings;
  mg_efu->hists = monitor.hists;

  std::shared_ptr<MGHitFile> dumpfile;
  if (!DetectorSettings.fileprefix.empty())
  {
    dumpfile = std::make_shared<MGHitFile>();
    dumpfile->open_rw(DetectorSettings.fileprefix + "mgmesytec_" + timeString() + ".h5");
  }
  mesytecdata = std::make_shared<MesytecData>(mg_efu, monitor.readouts,
                                              mg_config.spoof_high_time, dumpfile);

}

void CSPEC::mainThread() {
  init_config();

  /** Connection setup */
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(), EFUSettings.DetectorPort); //Change name or add more comments
  UDPReceiver cspecdata(local);
  cspecdata.setBufferSizes(EFUSettings.DetectorTxBufferSize, EFUSettings.DetectorRxBufferSize);
  cspecdata.printBufferSizes();
  cspecdata.setRecvTimeout(0, one_tenth_second_usecs); /// secs, usecs

  EV42Serializer flatbuffer(kafka_buffer_size, "multigrid");
  Producer EventProducer(EFUSettings.KafkaBroker, "C-SPEC_detector");
  flatbuffer.set_callback(std::bind(&Producer::produce2, &EventProducer, std::placeholders::_1));

  char buffer[eth_buffer_size];
  int ReadSize;
  TSCTimer report_timer;
  for (;;) {
    if ((ReadSize = cspecdata.receive(buffer, eth_buffer_size)) > 0) {
      mystats.rx_packets++;
      mystats.rx_bytes += ReadSize;
      XTRACE(INPUT, DEB, "read size: %u", ReadSize);

      auto res = mesytecdata->parse(buffer, ReadSize, flatbuffer);
      if (res != MesytecData::error::OK) {
        mystats.parse_errors++;
      }

      mystats.rx_readouts += mesytecdata->stats.readouts;
      mystats.rx_discards += mesytecdata->stats.discards;
      mystats.triggers += mesytecdata->stats.triggers;
      mystats.badtriggers += mesytecdata->stats.badtriggers;
      mystats.geometry_errors+= mesytecdata->stats.geometry_errors;
      mystats.tx_bytes += mesytecdata->stats.tx_bytes;
      mystats.rx_events += mesytecdata->stats.events;
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
