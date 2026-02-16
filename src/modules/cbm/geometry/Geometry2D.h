// Copyright (C) 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Geometry class for CBM 2D beam monitors with readout validation
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <logical_geometry/ESSGeometry.h>
#include <modules/cbm/geometry/Geometry.h>
#include <modules/cbm/geometry/Config.h>
#include <modules/cbm/readout/Parser.h>

#include <cstdint>
#include <string>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace cbm {

using namespace geometry;

///
/// \brief Geometry class for CBM 2D beam monitors (EVENT_2D type)
///
/// This class provides geometry handling and validation for EVENT_2D type
/// beam monitors. It validates X/Y coordinates against configured dimensions
/// and calculates pixel IDs using ESSGeometry.
///
class Geometry2D : public Geometry, public ESSGeometry {
public:
  // Public static metric name constants for CBM 2D-specific counters
  // clang-format off
  static inline const std::string METRIC_XPOS_ERRORS{"geometry.xpos_errors"};
  static inline const std::string METRIC_YPOS_ERRORS{"geometry.ypos_errors"};
  // clang-format on

  ///
  /// \brief CBM 2D-specific geometry statistics counters
  ///
  struct Geometry2DCounters : public StatCounterBase {
    int64_t XPosErrors{0};        ///< X position out of range errors
    int64_t YPosErrors{0};        ///< Y position out of range errors

    Geometry2DCounters(Statistics &Stats, const std::string &SourceName)
        : StatCounterBase(
              Stats, {{SourceName + "." + METRIC_XPOS_ERRORS, XPosErrors},
                      {SourceName + "." + METRIC_YPOS_ERRORS, YPosErrors}}) {}
  };

  ///
  /// \brief Constructor
  ///
  /// \param Stats Reference to Statistics object for counter registration
  /// \param Conf Reference to CBM configuration (MaxRing and MaxFENId are read
  /// from here) 
  /// \param MonitorType Beam monitor sub type, IBM, EVENT_0D, EVENT_2D
  /// \param SourceName Name of this geometry (e.g., "cbm7") for
  /// metric prefixing 
  /// \param Width Width of the 2D beam monitor (X dimension)
  /// \param Height Height of the 2D beam monitor (Y dimension)
  ///
  Geometry2D(Statistics &Stats, const Config &Conf, const CbmType &MonitorType,
             const std::string &SourceName, uint16_t Width, uint16_t Height)
      : Geometry(Stats, MonitorType, SourceName, 
           Conf.CbmParms.MaxRing, Conf.CbmParms.MaxFENId, Conf.CbmParms.MonitorRing,
          Width, Height), ESSGeometry(Width, Height, 1, 1), Cbm2DStats(Stats, SourceName), 
        Conf(Conf) {}

  ///
  /// \brief Validate readout data for a 2D beam monitor
  ///
  /// Performs comprehensive validation of readout data in the following order:
  /// 1. Type validation - checks if beam monitor type is correct
  /// 2. Ring validation - checks if Ring (calculated from FiberId) is within
  /// [0, MaxRing]
  /// 3. MonitorRing validation - checks if Ring matches configured MonitorRing
  /// 4. FEN validation - checks if FENId is within [0, MaxFEN]
  /// 4. Topology validation - verifies FEN/Channel combination exists in
  /// configuration
  /// 6. X coordinate validation - checks if XPos is within [0, Width)
  /// 7. Y coordinate validation - checks if YPos is within [0, Height)
  ///
  /// Validation stops at the first failure (short-circuit evaluation) and
  /// increments the appropriate error counter. The ValidationErrors counter is
  /// incremented for any validation failure.
  ///
  /// \param Data CbmReadout to validate
  /// \return true if all validations pass, false if any validation fails
  ///
  bool validateReadoutData(const Parser::CbmReadout &Data) const override {
    int Ring = calcRing(Data.FiberId);

    return validateAll(
        // 1. CBM Type Validation
        [&]() { return validateMonitorType(Data.Type); },
        // 2. MaxRing limit validation
        [&]() { return validateRing(Ring); },
        // 3. MonitorRing validation
        [&]() { return validateMonitorRing(Ring); },
        // 4. MaxFEN limit validation
        [&]() { return validateFEN(Data.FENId); },
        // 5. Topology validation
        [&]() {
          return validateTopology(*Conf.TopologyMapPtr, Data.FENId,
                                  Data.Channel);
        },
        // 6. 2D coordinate validations
        [&]() { return validateCoordinates(Data.Pos.XPos, Data.Pos.YPos) ; });
  }

protected:
  ///
  /// \brief Implementation of pixel calculation for CBM 2D geometry
  ///
  /// \note
  /// Assumes coordinates have been validated. Calculates pixel ID using
  /// ESSGeometry::pixel2D. Validation happens before this is called in the
  /// instrument.
  ///
  /// \param Data Const reference to CbmReadout object
  /// \return Calculated pixel ID, or 0 if coordinates are invalid
  ///
  uint32_t calcPixelImpl(const Parser::CbmReadout &Data) const override {
    return ESSGeometry::pixel2D(Data.Pos.XPos, Data.Pos.YPos);
  }

private:
  mutable Geometry2DCounters
      Cbm2DStats;               ///< CBM 2D-specific statistics counters
  const Config &Conf;           ///< Configuration reference
};

} // namespace cbm
