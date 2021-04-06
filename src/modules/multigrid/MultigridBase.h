// Copyright (C) 2018-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Multigrid detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include <common/Detector.h>
#include <common/EV42Serializer.h>
#include <multigrid/Config.h>
#include <common/monitor/Monitor.h>

struct MultigridSettings {
  std::string ConfigFile;
  std::string FilePrefix;
  // \todo move this to json
  bool monitor{false};
};

///
class MultigridBase : public Detector {
public:
  MultigridBase(BaseSettings const &settings, MultigridSettings const &LocalSettings);
  ~MultigridBase() = default;
  void mainThread();

  /// Some hardcoded constants
  static constexpr int one_tenth_second_usecs{100000}; ///

protected:

  struct {
    // Input Counters
    int64_t rx_packets{0};
    int64_t rx_bytes{0};
    int64_t rx_idle{0};
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
    int64_t tx_events{0};
    int64_t tx_bytes{0};
    // Kafka stats below are common to all detectors
    int64_t kafka_produce_fails{0};
    int64_t kafka_ev_errors{0};
    int64_t kafka_ev_others{0};
    int64_t kafka_dr_errors{0};
    int64_t kafka_dr_noerrors{0};
  } __attribute__((aligned(64))) Counters;

  MultigridSettings ModuleSettings;
  Multigrid::Config mg_config;
  Monitor monitor;

  bool HavePulseTime{false};
  uint64_t ShortestPulsePeriod{std::numeric_limits<uint64_t>::max()};

  bool init_config();
  void process_events(EV42Serializer &ev42serializer);

};
