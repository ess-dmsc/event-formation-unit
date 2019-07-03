/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <logical_geometry/ESSGeometry.h>
#include <gdgem/srs/SRSMappings.h>
#include <gdgem/srs/SRSTime.h>
#include <common/reduction/AbstractAnalyzer.h>
#include <gdgem/srs/CalibrationFile.h>
#include <memory>
#include <string>

namespace Gem {

struct ClustererConfig {
  uint16_t max_strip_gap{2};
  double max_time_gap{200};
};

struct EventFilter {

  bool enforce_lower_uncertainty_limit{false};
  int16_t lower_uncertainty_limit{6};
  size_t lower_uncertainty_dropped{0};

  bool enforce_minimum_hits{false};
  uint32_t minimum_hits{6};
  size_t minimum_hits_dropped{0};

  bool valid(Event &event, const ReducedEvent& utpc);
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

  bool perform_clustering {true};
  bool send_raw_hits {false};

  ClustererConfig clusterer_x;
  ClustererConfig clusterer_y;

  //matcher
  double matcher_max_delta_time{200};

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
