/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief MGMesytec detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include <common/Detector.h>
#include <common/Hists.h>
#include <common/HistSerializer.h>
#include <common/Log.h>
#include <common/ReadoutSerializer.h>
#include <multigrid/MgConfig.h>

struct MGMesytecSettings {
  std::string ConfigFile;
  bool monitor{false};
  std::string FilePrefix;
};

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
    readouts->set_callback(std::bind(&Producer::produce2<uint8_t>, producer.get(), std::placeholders::_1));
    histfb->set_callback(std::bind(&Producer::produce2<uint8_t>, producer.get(), std::placeholders::_1));
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
      LOG(PROCESS, Sev::Debug, "Flushing histograms for {} readouts", hists->hit_count());
      histfb->produce(*hists);
      hists->clear();
    }

    if (readouts->getNumEntries()) {
      LOG(PROCESS, Sev::Debug, "Flushing readout data for {} readouts", readouts->getNumEntries());
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



///
class MGMesytecBase : public Detector {
public:
  MGMesytecBase(BaseSettings const &settings, struct MGMesytecSettings &LocalMGMesytecSettings);
  ~MGMesytecBase() = default;
  void mainThread();

  /// Some hardcoded constants
  static constexpr int eth_buffer_size {9000};          /// used for experimentation
  static constexpr size_t kafka_buffer_size {1000000};  /// -||-
  static constexpr size_t readout_entries {100000};     /// number of raw readout entries
  static constexpr int one_tenth_second_usecs {100000}; ///

protected:

  struct {
    // Input Counters
    int64_t rx_packets {0};
    int64_t rx_bytes {0};
    int64_t discarded_bytes {0};
    int64_t triggers {0};
    int64_t bus_glitches {0};
    int64_t bad_triggers {0};
    int64_t readouts {0};
    int64_t readouts_discarded {0};
    int64_t readouts_culled {0};
    int64_t geometry_errors {0};
    int64_t timing_errors {0};
    int64_t events {0};
    int64_t tx_bytes {0};
  } __attribute__((aligned(64))) mystats;

  void init_config();

  struct MGMesytecSettings MGMesytecSettings;
  Multigrid::Config mg_config;
  Monitor monitor;

  bool HavePulseTime{false};
  uint64_t ShortestPulsePeriod{std::numeric_limits<uint64_t>::max()};
};
