// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Intermediate geometry class for DREAM-type detectors providing
/// unified validation and counter management
///
/// This class organizes DREAM detector geometry validation through the
/// DetectorGeometry base class framework, ensuring proper counter increment
/// for validation errors. It provides methods to validate Ring, FEN, and
/// configuration status before pixel calculation.
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <common/geometry/DetectorGeometry.h>
#include <cstdint>
#include <modules/dream/geometry/Config.h>
#include <modules/dream/readout/DataParser.h>

#include <string>
#include <typeinfo>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Dream {

using namespace geometry;

class Geometry : public DetectorGeometry<DataParser::CDTReadout> {

private:
  struct DreamGeometryCounters : public StatCounterBase {
    int64_t ConfigErrors{0};

    DreamGeometryCounters(Statistics &Stats)
        : StatCounterBase(Stats,
                          {{METRIC_COUNTER_CONFIG_ERRORS, ConfigErrors}}) {}
  };

public:
  // Counter name constants
  // clang-format off
  static inline const std::string METRIC_COUNTER_CONFIG_ERRORS = "geometry.config_errors";
  // clang-format on

  /// \brief Constructor
  /// \param Stats Reference to Statistics object for counter registration
  /// \param Config Reference to the DREAM configuration object
  Geometry(Statistics &Stats, const Config &Config)
      : DetectorGeometry(Stats, Config.MaxRing, Config.MaxFEN),
        DreamCounters(Stats), DreamConfig(Config) {}

  /// \brief Geometry statistics counters with automatic registration
  ///        limited to DREAM-specific geometry objects.
  mutable DreamGeometryCounters DreamCounters; ///< Geometry statistics counters
  const Config &DreamConfig; ///< Reference to DREAM configuration

  /// \brief Validates the readout data based on instrument configuration and
  /// parameters
  /// \param Data CDTReadout to check validity of.
  /// \return true if valid, false otherwise
  virtual bool validateReadoutData(const DataParser::CDTReadout &Data) {
    int Ring = calcRing(Data.FiberId);

    return validateAll(
        [&]() { return validateRing(Ring); },
        [&]() { return validateFEN(Data.FENId); },
        [&]() { return validateConfigMapping(Ring, Data.FENId); });
  }

  /// \brief Get access to DREAM-specific geometry statistics counters
  inline const DreamGeometryCounters &getDreamCounters() const {
    return DreamCounters;
  }

protected:
  /// \brief Extract ModuleParms from configuration based on Ring and FENId
  /// \param Ring Ring index
  /// \param FENId FEN index
  /// \return Reference to ModuleParms for the given Ring/FEN
  const Config::ModuleParms &getModuleParms(int Ring, int FENId) const {
    return DreamConfig.RMConfig[Ring][FENId];
  }

  /// \brief Validate that configuration for Ring/FEN is initialized
  /// \param Ring Ring index to check
  /// \param FEN FEN index to check
  /// \return true if initialized, false otherwise
  inline bool validateConfigMapping(int Ring, int FEN) const {
    if (Ring < 0 || Ring >= DreamConfig.MaxRing || FEN < 0 ||
        FEN >= DreamConfig.MaxFEN) {
      XTRACE(DATA, WAR, "Ring/FEN out of range: Ring=%d, FEN=%d", Ring, FEN);
      DreamCounters.ConfigErrors++;
      return false;
    }

    const Config::ModuleParms &Parms = DreamConfig.RMConfig[Ring][FEN];
    if (!Parms.Initialised) {
      XTRACE(DATA, WAR, "Configuration not initialized for Ring %d, FEN %d",
             Ring, FEN);
      DreamCounters.ConfigErrors++;
      return false;
    }
    return true;
  }
};
} // namespace Dream
