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
#include <common/clustering/Event.h>
#include <gdgem/nmx/uTPC.h>
#include <gdgem/vmm3/CalibrationFile.h>
#include <memory>
#include <string>

namespace Gem {

struct ClustererConfig {
  uint16_t hit_adc_threshold{0};
  uint16_t max_strip_gap{2};
  double max_time_gap{200};
  size_t min_cluster_size{3};
};

struct EventFilter {

  bool enforce_lower_uncertainty_limit{false};
  int16_t lower_uncertainty_limit{6};
  size_t lower_uncertainty_dropped{0};

  bool enforce_minimum_hits{false};
  uint32_t minimum_hits{6};
  size_t minimum_hits_dropped{0};

  /// \todo bug? uncertainty takes precedence if both enforce options are true
  bool valid(Event &event, const utpcResults& utpc) {
    if (enforce_lower_uncertainty_limit &&
        !utpcAnalyzer::meets_lower_criterion(utpc.x, utpc.y, lower_uncertainty_limit)) {
      lower_uncertainty_dropped++;
      return false;
    }
    if (enforce_minimum_hits &&
            ((event.c1.hit_count() < minimum_hits) ||
                (event.c2.hit_count() < minimum_hits))) {
      minimum_hits_dropped++;
      return false;
    }
    return true;
  }
};

struct NMXConfig {
  NMXConfig() {}
  NMXConfig(std::string configfile, std::string calibrationfile);

  std::string builder_type{"VMM3"};

  // VMM calibration
  std::shared_ptr<CalibrationFile> calfile;

  // SRS only
  SRSTime time_config;
  SRSMappings srs_mappings;

  ClustererConfig clusterer_x;
  ClustererConfig clusterer_y;

  //matcher
  double matcher_max_delta_time{200};

  // analysis
  bool analyze_weighted{true};
  int16_t analyze_max_timebins{3};
  int16_t analyze_max_timedif{7};

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
