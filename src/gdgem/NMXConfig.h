/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <logical_geometry/ESSGeometry.h>
#include <gdgem/srs/SRSMappings.h>
#include <gdgem/srs/SRSTime.h>
#include <common/reduction/analysis/AbstractAnalyzer.h>
#include <gdgem/srs/CalibrationFile.h>
#include <memory>

namespace Gem {

struct ClustererConfig {
  uint16_t max_strip_gap{2};
  double max_time_gap{200};
};

struct EventFilter {
  bool enforce_minimum_hits{false};
  uint32_t minimum_hits{6};
  float plane_0_vs_1_ratio_max{10};
  float plane_0_vs_1_ratio_min{0.1};
  bool enforce_charge_ratio{false};
  size_t minimum_hits_dropped{0};
  size_t charge_ratio_dropped{0};

  bool valid(Event &event);
};

struct NMXConfig {
  NMXConfig() = default;
  NMXConfig(std::string configfile, std::string calibrationfile);

  std::string builder_type;

  // VMM calibration
  std::shared_ptr<CalibrationFile> calfile;

  // SRS only
  SRSTime time_config;
  SRSMappings srs_mappings;

  uint16_t adc_threshold{0};

  bool enable_data_processing {true};
  bool perform_clustering {true};
  bool send_raw_hits {false};

  ClustererConfig clusterer_x;
  ClustererConfig clusterer_y;

  //matcher
  double matcher_max_delta_time{200};
  //Matcher algorithm, either center-of-mass, charge2, utpc, or utpc_weighted
  std::string time_algorithm{"center-of-mass"};
  
  // Name of the event analyzer
  std::string analyzer_name{"EventAnalyzer"};
  
  // Name of the matcher
  std::string matcher_name{"CenterMatcher"};
  
  // Name of the clusterer
  std::string clusterer_name{"GapClusterer"};

  // analysis
  std::shared_ptr<AbstractAnalyzer> analyzer_;

  // filtering
  EventFilter filter;

  // Monitor
  bool hit_histograms{false};
  uint32_t cluster_adc_downshift{6};
  bool send_tracks{false};
  size_t track_sample_minhits{6};

  // Event formation
  ESSGeometry geometry;

  std::string debug() const;
};

}
