// Copyright (C) 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Base class for specialized CBM geometry classes
///
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/geometry/DetectorGeometry.h>
#include <modules/cbm/CbmTypes.h>
#include <modules/cbm/readout/Parser.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace cbm {

using namespace geometry;

///
/// \brief Base class for CBM geometries used for readout validation
///
/// Base class for CBM geometries used for readout validation
class Geometry : public DetectorGeometry<Parser::CbmReadout>  {
protected:
  // Public static metric name constants for CBM base geometry counters
  // clang-format off
  static inline const std::string METRIC_MONITOR_RING_MISMATCH_ERRORS{"geometry.monitor_ring_mismatch_errors"};
  static inline const std::string METRIC_TYPE_MISMATCH_ERRORS{"geometry.type_mismatch_errors"};
  // Public static metric name constants for CBM 2D-specific counters
  static inline const std::string METRIC_XPOS_ERRORS{"geometry.xpos_errors"};
  static inline const std::string METRIC_YPOS_ERRORS{"geometry.ypos_errors"};
  // clang-format on

  ///
  /// \brief CBM base geometry statistics counters
  ///
  struct CbmGeometryCounters : public StatCounterBase {
    int64_t MonitorRingMismatchErrors{
        0};                       ///< Ring does not match configured MonitorRing
    int64_t TypeMismatchError{0};  ///< Monitory Type does not match configured Type
    // Public static metric name constants for CBM 2D-specific counters
    int64_t XPosErrors{0};        ///< X position out of range errors
    int64_t YPosErrors{0};        ///< Y position out of range errors

    CbmGeometryCounters(Statistics &Stats, const std::string &SourceName)
      : StatCounterBase(Stats,
          {{SourceName + "." + METRIC_MONITOR_RING_MISMATCH_ERRORS,
            MonitorRingMismatchErrors},
            {SourceName + "." + METRIC_TYPE_MISMATCH_ERRORS, 
              TypeMismatchError},
            {SourceName + "." + METRIC_XPOS_ERRORS, XPosErrors},
            {SourceName + "." + METRIC_YPOS_ERRORS, YPosErrors}}) {}
  };

  /// \brief Constructor. Used by 2D geometry
  /// \param Stats Reference to Statistics object for counter registration
  /// \param MonitorType Beam monitor sub type, IBM, EVENT_0D, EVENT_2D
  /// \param SourceName Name of this geometry (e.g., "cbm1") for metric prefixing 
  /// \param MaxRing Maximum valid Ring value for this detector
  /// \param MaxFEN Maximum valid FEN value for this detector
  /// \param MonitorRing CBM monitor ring
  /// \param Width Width of the 2D beam monitor (X dimension)
  /// \param Height Height of the 2D beam monitor (Y dimension)
  explicit Geometry(Statistics &Stats, const CbmType &MonitorType, const std::string &SourceName, 
          int MaxRing, int MaxFEN, uint8_t MonitorRing, uint16_t Width, uint16_t Height)
    : DetectorGeometry{Stats, MaxRing, MaxFEN}, CbmStats(Stats, SourceName) 
    , MonitorType{MonitorType}, SourceName{SourceName}, MonitorRing{MonitorRing}
    , Width(Width), Height(Height) {}


  /// \brief Constructor. Used by 0D geometry
  /// \param Stats Reference to Statistics object for counter registration
  /// \param MonitorType Beam monitor sub type, IBM, EVENT_0D, EVENT_2D
  /// \param SourceName Name of this geometry (e.g., "cbm1") for metric prefixing 
  /// \param MaxRing Maximum valid Ring value for this detector
  /// \param MaxFEN Maximum valid FEN value for this detector
  /// \param MonitorRing CBM monitor ring
  explicit Geometry(Statistics &Stats, const CbmType &MonitorType, const std::string &SourceName, 
        int MaxRing, int MaxFEN, uint8_t MonitorRing)
    : Geometry{Stats, MonitorType, SourceName, MaxRing, MaxFEN, MonitorRing, 0, 0} {}
      
public:

  /// \brief Get access to CBM-specific geometry statistics counters
  inline const CbmGeometryCounters &getGeometryCounters() const {
    return CbmStats;
  }

  /// \brief MonitorRing validation - checks if Ring matches configured MonitorRing
  /// \param Ring monitor ring value
  /// \return true if value is equal to configure
  inline bool validateMonitorRing(int Ring) const {
    if (MonitorRing == Ring) {
      return true;
    }
    XTRACE(DATA, WAR,
            "Readout Ring %d does not match "
            "configured MonitorRing %d",
            Ring, MonitorRing);
    CbmStats.MonitorRingMismatchErrors++;
    return false;
  }

  /// \brief Type validation - checks if beam monitor type is correct
  /// \param Type monitor type value
  /// \return true if value is equal to configure
  inline bool validateMonitorType(int Type) const {
    if (Type == static_cast<uint8_t>(MonitorType)) {
      return true;
    } 
    XTRACE(DATA, WAR,
      "Monitor Type %d does not match "
      "configured Type %d",
      Type, MonitorType);
      CbmStats.TypeMismatchError++;
    return false;
  }

  /// \brief Coordinate validation - checks if XPos is within [0, Width) 
  /// and YPos is within [0, Height)
  /// \param XPos x-coordinates
  /// \param YPos y-coordinates
  /// \return true if values are with in limits
  inline bool validateCoordinates(uint16_t XPos, uint16_t YPos) const {
    if (XPos >= Width) {
      XTRACE(DATA, WAR, "XPos %u exceeds width %u", XPos, Width);
      CbmStats.XPosErrors++;
      return false;
    }
    if (YPos >= Height) {
      XTRACE(DATA, WAR, "YPos %u exceeds height %u", YPos,
              Height);
      CbmStats.YPosErrors++;
      return false;
    }
    return true;
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

private:

  /// \brief Geometry statistics counters with automatic registration
  ///        limited to CBM-specific geometry objects.
  mutable CbmGeometryCounters CbmStats; ///< Geometry statistics counters
  CbmType MonitorType;
  const std::string SourceName; ///< Source name for metric prefixing
  uint8_t MonitorRing{0};       ///< Monitor ring
  const uint16_t Width{0};      ///< Width of the 2D beam monitor
  const uint16_t Height{0};     ///< Height of the 2D beam monitor
};
} // namespace cbm
