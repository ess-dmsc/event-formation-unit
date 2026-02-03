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
#include <common/geometry/DetectorGeometry.h>
#include <logical_geometry/ESSGeometry.h>
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
class Geometry2D : public DetectorGeometry<Parser::CbmReadout>,
                   public ESSGeometry {
public:
  // Public static metric name constants for CBM 2D-specific counters
  // clang-format off
  static inline const std::string METRIC_XPOS_ERRORS{"geometry.xpos_errors"};
  static inline const std::string METRIC_YPOS_ERRORS{"geometry.ypos_errors"};
  static inline const std::string METRIC_MONITOR_RING_MISMATCH_ERRORS{"geometry.monitor_ring_mismatch_errors"};
  // clang-format on

  ///
  /// \brief CBM 2D-specific geometry statistics counters
  ///
  struct Geometry2DCounters : public StatCounterBase {
    int64_t XPosErrors{0}; ///< X position out of range errors
    int64_t YPosErrors{0}; ///< Y position out of range errors
    int64_t MonitorRingMismatchErrors{
        0}; ///< Ring does not match configured MonitorRing

    Geometry2DCounters(Statistics &Stats, const std::string &SourceName)
        : StatCounterBase(
              Stats, {{SourceName + "." + METRIC_XPOS_ERRORS, XPosErrors},
                      {SourceName + "." + METRIC_YPOS_ERRORS, YPosErrors},
                      {SourceName + "." + METRIC_MONITOR_RING_MISMATCH_ERRORS,
                       MonitorRingMismatchErrors}}) {}
  };

  ///
  /// \brief Constructor
  ///
  /// \param Stats Reference to Statistics object for counter registration
  /// \param Conf Reference to CBM configuration (MaxRing and MaxFENId are read
  /// from here) 
  /// \param SourceName Name of this geometry (e.g., "cbm7") for
  /// metric prefixing 
  /// \param Width Width of the 2D beam monitor (X dimension)
  /// \param Height Height of the 2D beam monitor (Y dimension)
  ///
  Geometry2D(Statistics &Stats, const Config &Conf,
             const std::string &SourceName, uint16_t Width, uint16_t Height)
      : DetectorGeometry(Stats, Conf.CbmParms.MaxRing, Conf.CbmParms.MaxFENId),
        ESSGeometry(Width, Height, 1, 1), Cbm2DStats(Stats, SourceName),
        Conf(Conf), SourceName(SourceName), Width(Width), Height(Height) {}

  ///
  /// \brief Validate readout data for a 2D beam monitor
  ///
  /// Performs comprehensive validation of readout data in the following order:
  /// 1. Ring validation - checks if Ring (calculated from FiberId) is within
  /// [0, MaxRing]
  /// 2. FEN validation - checks if FENId is within [0, MaxFEN]
  /// 3. Topology validation - verifies FEN/Channel combination exists in
  /// configuration
  /// 4. MonitorRing validation - checks if Ring matches configured MonitorRing
  /// 5. X coordinate validation - checks if XPos is within [0, Width)
  /// 6. Y coordinate validation - checks if YPos is within [0, Height)
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
        // 1. MaxRing limit validation
        [&]() { return validateRing(Ring); },
        // 2. MonitorRing validation
        [&]() {
          if (Conf.CbmParms.MonitorRing != Ring) {
            XTRACE(DATA, WAR,
                   "Readout Ring %d does not match "
                   "configured MonitorRing %d",
                   Ring, Conf.CbmParms.MonitorRing);
            Cbm2DStats.MonitorRingMismatchErrors++;
            return false;
          }
          return true;
        },
        // 3. MaxFEN limit validation
        [&]() { return validateFEN(Data.FENId); },
        [&]() {
          return validateTopology(*Conf.TopologyMapPtr, Data.FENId,
                                  Data.Channel);
        },
        // 4. 2D coordinate validations
        [&]() {
          if (Data.Pos.XPos >= Width) {
            XTRACE(DATA, WAR, "XPos %u exceeds width %u", Data.Pos.XPos, Width);
            Cbm2DStats.XPosErrors++;
            return false;
          }
          return true;
        },
        // 5. 2D coordinate validations
        [&]() {
          if (Data.Pos.YPos >= Height) {
            XTRACE(DATA, WAR, "YPos %u exceeds height %u", Data.Pos.YPos,
                   Height);
            Cbm2DStats.YPosErrors++;
            return false;
          }
          return true;
        });
  }

  ///
  /// \brief Get access to CBM 2D-specific geometry statistics counters
  ///
  inline const Geometry2DCounters &getGeometryCounters() const {
    return Cbm2DStats;
  }

  ///
  /// \brief Get the source name for this geometry
  ///
  inline const std::string &getSourceName() const { return SourceName; }

  ///
  /// \brief Get the configured width
  ///
  inline uint16_t getWidth() const { return Width; }

  ///
  /// \brief Get the configured height
  ///
  inline uint16_t getHeight() const { return Height; }

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
  const std::string SourceName; ///< Source name for metric prefixing
  const uint16_t Width;         ///< Width of the 2D beam monitor
  const uint16_t Height;        ///< Height of the 2D beam monitor
};

} // namespace cbm
