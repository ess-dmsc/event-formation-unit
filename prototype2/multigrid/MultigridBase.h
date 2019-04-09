/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Multigrid detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include <common/Detector.h>
#include <common/Hists.h>
#include <common/HistSerializer.h>
#include <common/Log.h>
#include <common/ReadoutSerializer.h>
#include <common/EV42Serializer.h>
#include <multigrid/Config.h>

struct MultigridSettings {
  std::string ConfigFile;
  std::string FilePrefix;
  // \todo move this to json
  bool monitor{false};
};

struct Monitor {
  std::shared_ptr<Hists> hists;
  std::shared_ptr<ReadoutSerializer> readouts;

  void init(std::string broker, size_t max_readouts) {
    readouts = std::make_shared<ReadoutSerializer>(max_readouts, "multigrid");
    hists = std::make_shared<Hists>(std::numeric_limits<uint16_t>::max(),
                                    std::numeric_limits<uint16_t>::max());
    histfb = std::make_shared<HistSerializer>(hists->needed_buffer_size(), "multigrid");

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
  bool enabled_{false};

  std::shared_ptr<Producer> producer;
  std::shared_ptr<HistSerializer> histfb;
};

///
class MultigridBase : public Detector {
public:
  MultigridBase(BaseSettings const &settings, MultigridSettings const &LocalSettings);
  ~MultigridBase() = default;
  void mainThread();

  /// Some hardcoded constants
  static constexpr int eth_buffer_size{9000};          /// used for experimentation
  static constexpr size_t kafka_buffer_size{1000000};  /// -||-
  static constexpr size_t readout_entries{100000};     /// number of raw readout entries
  static constexpr int one_tenth_second_usecs{100000}; ///

protected:

  struct {
    // Input Counters
    int64_t rx_packets{0};
    int64_t rx_bytes{0};
    int64_t readouts_total{0};
    int64_t parser_discarded_bytes{0};
    int64_t parser_triggers{0};
    int64_t builder_glitch_rejects{0};
    int64_t builder_filter_rejects{0};
    int64_t builder_geometry_errors{0};
    int64_t hits_total{0};
    int64_t hits_time_seq_err{0};
    int64_t hits_bad_plane{0};
    int64_t hits_used{0};
    int64_t pulses{0};
    int64_t wire_clusters{0};
    int64_t grid_clusters{0};
    int64_t events_total{0};
    int64_t events_multiplicity_rejects{0};
    int64_t events_bad{0};
    int64_t events_geometry_err{0};
    int64_t events_time_err{0};
    int64_t tx_events{0};
    int64_t tx_bytes{0};
    // Kafka stats below are common to all detectors
    int64_t kafka_produce_fails{0};
    int64_t kafka_ev_errors{0};
    int64_t kafka_ev_others{0};
    int64_t kafka_dr_errors{0};
    int64_t kafka_dr_noerrors{0};
  } __attribute__((aligned(64))) mystats;

  MultigridSettings ModuleSettings;
  Multigrid::Config mg_config;
  Monitor monitor;

  bool HavePulseTime{false};
  uint64_t ShortestPulsePeriod{std::numeric_limits<uint64_t>::max()};

  void init_config();
  void process_events(EV42Serializer &ev42serializer);

};
