/** Copyright (C) 2017 European Spallation Source ERIC */

#pragma once

#include <cinttypes>
#include <gdgem/nmx/Geometry.h>
#include <gdgem/srs/SRSMappings.h>
#include <gdgem/srs/SRSTime.h>
#include <string>

struct NMXConfig {
  NMXConfig() {}
  NMXConfig(std::string jsonfile);

  std::string builder_type{"SRS"};

  // SRS only
  SRSTime time_config;
  SRSMappings srs_mappings;

  // analysis
  uint64_t cluster_min_timespan{30};
  bool analyze_weighted{true};
  int16_t analyze_max_timebins{3};
  int16_t analyze_max_timedif{7};

  bool enforce_lower_uncertainty_limit{false};
  int16_t lower_uncertainty_limit{6};
  bool enforce_minimum_eventlets{false};
  uint32_t minimum_eventlets{6};

  // Monitor
  uint32_t cluster_adc_downshift{6};
  size_t track_sample_minhits{6};

  // Event formation
  size_t geometry_x{256};
  size_t geometry_y{256};

  std::string debug() const;

  bool dump_csv{false};
  bool dump_h5{false};
  std::string dump_directory{};
};
