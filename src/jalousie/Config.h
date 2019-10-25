/// Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <jalousie/SumoMappings.h>
#include <jalousie/Readout.h>
#include <common/reduction/ChronoMerger.h>
#include <common/EV42Serializer.h>
#include <logical_geometry/ESSGeometry.h>

namespace Jalousie {

struct Config {
  Config() = default;
  Config(std::string jsonfile);

  /// Based on some post-analysis of data, or a-priori knowledge of beamline
  /// Current default value is based on 2019.07 V20 tests, about 3x maximum
  /// observed pulse period, which was 10416684
  /// CDT readout system uses 96ns clock
  uint64_t maximum_latency{30000000};

  /// Which board-id's map to which modules for mappings and merger
  /// Invalid result is == std::numeric_limits<size_t>::max()
  std::vector<size_t> board_mappings;

  /// SUMO mappings for each module
  std::vector<SumoMappings> SUMO_mappings;

  /// Logical geometry for the whole pipeline
  ESSGeometry geometry;

  /// Will be replaced upon configuration
  /// Has 0s just to make default initialization possible
  ChronoMerger merger {0,0};

  std::string debug() const;
};

}
