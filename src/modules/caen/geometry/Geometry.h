// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from tube and amplitudes
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <common/geometry/DetectorGeometry.h>
#include <cstdint>
#include <logical_geometry/ESSGeometry.h>
#include <modules/caen/geometry/CDCalibration.h>
#include <modules/caen/readout/DataParser.h>

#include <limits>
#include <string>
#include <sys/types.h>
#include <utility>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

using namespace geometry;

class Geometry : public DetectorGeometry<DataParser::CaenReadout> {
protected:
  // Counter name constants
  // clang-format off
  static inline const std::string METRIC_COUNTER_TOPOLOGY_ERRORS = "geometry.topology_errors";
  static inline const std::string METRIC_COUNTER_AMPL_ZERO = "geometry.ampl_zero";
  static inline const std::string METRIC_COUNTER_AMPL_LOW = "geometry.ampl_low";
  static inline const std::string METRIC_COUNTER_AMPL_HIGH = "geometry.ampl_high";
  static inline const std::string METRIC_COUNTER_GROUP_ERRORS = "geometry.group_errors";
  static inline const std::string METRIC_COUNTER_GLOBALPOS_INVALID = "geometry.globalpos_invalid";
  static inline const std::string METRIC_COUNTER_ZERODIV_ERROR = "geometry.zerodiv_error";
  static inline const std::string METRIC_COUNTER_UNITID_INVALID = "geometry.unitid_invalid";
  // clang-format on

  struct CaenGeometryCounters : public StatCounterBase {
    int64_t AmplitudeZero{0};
    int64_t AmplitudeLow{0};
    int64_t AmplitudeHigh{0};
    int64_t GroupErrors{0};
    int64_t GlobalPosInvalid{0};
    int64_t ZeroDivError{0};
    int64_t UnitIdInvalid{0};

    CaenGeometryCounters(Statistics &Stats)
        : StatCounterBase(Stats, {{METRIC_COUNTER_AMPL_ZERO, AmplitudeZero},
                                  {METRIC_COUNTER_AMPL_LOW, AmplitudeLow},
                                  {METRIC_COUNTER_AMPL_HIGH, AmplitudeHigh},
                                  {METRIC_COUNTER_GROUP_ERRORS, GroupErrors},
                                  {METRIC_COUNTER_GLOBALPOS_INVALID, GlobalPosInvalid},
                                  {METRIC_COUNTER_ZERODIV_ERROR, ZeroDivError},
                                  {METRIC_COUNTER_UNITID_INVALID, UnitIdInvalid}}) {
    }
  };

  /// \brief Constructor
  /// \param Stats Reference to Statistics object for counter registration
  /// \param MaxRing Maximum valid Ring value for this detector
  /// \param MaxFEN Maximum valid FEN value for this detector
  /// \param MaxGroup Maximum valid Group value for this detector
  /// \param MaxAmpl Maximum valid amplitude value for this detector
  Geometry(Statistics &Stats, int MaxRing, int MaxFEN, int MaxGroup,
           int MaxAmpl = std::numeric_limits<int>::max())
      : DetectorGeometry(Stats, MaxRing, MaxFEN), 
        CaenStats(Stats), MaxGroup(MaxGroup), MaxAmpl(MaxAmpl) {}

  /// \brief Geometry statistics counters with automatic registration
  ///        limited to CAEN-specific geometry objects.
  mutable CaenGeometryCounters CaenStats; ///< Geometry statistics counters

public:
  /// \brief sets the calibration parameters for straw stretch corrections
  /// \param Calib Calibration object containing polynomial correction values
  void setCalibration(const CDCalibration &Calib) { CaenCDCalibration = Calib; }

  /// \brief returns true if Data is a valid readout with the given config
  /// \param Data CaenReadout to check validity of.
  virtual bool validateReadoutData(const DataParser::CaenReadout &Data) const = 0;

  /// \brief return the total number of serializers used by the geometry
  [[nodiscard]] virtual size_t numSerializers() const = 0;

  /// \brief calculate the serializer index for the given readout
  /// \param Data CaenReadout to calculate serializer index for
  [[nodiscard]] virtual size_t
  calcSerializer(const DataParser::CaenReadout &Data) const = 0;

  /// \brief return the name of the serializer at the given index
  [[nodiscard]] virtual std::string serializerName(size_t Index) const = 0;

  /// \brief Get access to CAEN-specific geometry statistics counters
  inline const CaenGeometryCounters &getCaenCounters() const {
    return CaenStats;
  }

  inline bool validateGroup(int Group) const {
    if (Group < 0 || Group > MaxGroup) {
      XTRACE(DATA, WAR, "Group %d is invalid (out of range)", Group);
      CaenStats.GroupErrors++;
      return false;
    }
    return true;
  }

  /// \brief Validate amplitude sum is not zero
  /// \param AmpA Amplitude A value
  /// \param AmpB Amplitude B value
  /// \return true if sum is non-zero, false otherwise
  inline bool validateAmplitudeZero(int AmpA, int AmpB) const {
    if (AmpA + AmpB == 0) {
      XTRACE(DATA, DEB, "Sum of amplitudes is 0");
      CaenStats.AmplitudeZero++;
      return false;
    }
    return true;
  }

  /// \brief Validate amplitude sum does not exceed maximum
  /// \param AmpA Amplitude A value
  /// \param AmpB Amplitude B value
  /// \return true if sum is within limits, false otherwise
  inline bool validateAmplitudeHigh(int AmpA, int AmpB) const {
    if (AmpA + AmpB > MaxAmpl) {
      XTRACE(DATA, DEB, "Sum of amplitudes exceeds maximum");
      CaenStats.AmplitudeHigh++;
      return false;
    }
    return true;
  }

  /// \brief Validate individual amplitudes meet minimum threshold
  /// \param AmpA Amplitude A value
  /// \param AmpB Amplitude B value
  /// \param MinAmpl Minimum valid amplitude
  /// \return true if both amplitudes meet minimum, false otherwise
  inline bool validateAmplitudeLow(int AmpA, int AmpB, int MinAmpl) const {
    if ((AmpA < MinAmpl) || (AmpB < MinAmpl)) {
      XTRACE(DATA, DEB, "At least one amplitude is too low");
      CaenStats.AmplitudeLow++;
      return false;
    }
    return true;
  }

  CDCalibration CaenCDCalibration;
  const int MaxGroup{0}; ///< Maximum valid Group value (exclusive)
  const int MinAmpl{0};      ///< Minimum valid amplitude value
  const int MaxAmpl{
      std::numeric_limits<int>::max()}; ///< Maximum valid amplitude value
  // Note: per-detector resolution (previously UnitResolution) has been
  // moved into each concrete geometry implementation as `Resolution`.
};
} // namespace Caen
