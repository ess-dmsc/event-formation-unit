// Copyright (C) 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Base geometry class for CBM non-2D beam monitors (EVENT_0D and IBM)
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <common/geometry/DetectorGeometry.h>
#include <modules/cbm/geometry/Config.h>
#include <modules/cbm/readout/Parser.h>

#include <cstdint>
#include <string>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace cbm {

using namespace geometry;

///
/// \brief Base geometry class for CBM non-2D beam monitors (EVENT_0D and IBM)
///
/// This class provides geometry handling and validation for EVENT_0D and IBM
/// type beam monitors. These monitor types don't have 2D position data but
/// still require Ring/FEN/Topology validation based on fiber cabling.
///
/// For EVENT_0D: Returns a fixed pixel offset from the topology configuration
/// For IBM: Returns 0 (pixel not used, histogram uses ADC values instead)
///
class Geometry0D : public DetectorGeometry<Parser::CbmReadout> {
public:
  // Public static metric name constants for CBM base geometry counters
  // clang-format off
  static inline const std::string METRIC_MONITOR_RING_MISMATCH_ERRORS{"geometry.monitor_ring_mismatch_errors"};
  // clang-format on

  ///
  /// \brief CBM base geometry statistics counters
  ///
  struct CbmBaseGeometryCounters : public StatCounterBase {
    int64_t MonitorRingMismatchErrors{
        0}; ///< Ring does not match configured MonitorRing

    CbmBaseGeometryCounters(Statistics &Stats, const std::string &SourceName)
        : StatCounterBase(Stats,
                          {{SourceName + "." + METRIC_MONITOR_RING_MISMATCH_ERRORS,
                            MonitorRingMismatchErrors}}) {}
  };

  ///
  /// \brief Constructor
  ///
  /// \param Stats Reference to Statistics object for counter registration
  /// \param Conf Reference to CBM configuration (MaxRing and MaxFENId are read
  /// from here) 
  /// \param SourceName Name of this geometry (e.g., "cbm1") for
  /// metricprefixing 
  /// \param PixelOffset Fixed pixel value to return for
  /// EVENT_0D (0 for IBM)
  ///
  Geometry0D(Statistics &Stats, const Config &Conf,
             const std::string &SourceName, uint32_t PixelOffset = 0)
      : DetectorGeometry(Stats, Conf.CbmParms.MaxRing, Conf.CbmParms.MaxFENId),
        CbmBaseStats(Stats, SourceName), Conf(Conf), SourceName(SourceName),
        PixelOffset(PixelOffset) {}

  ///
  /// \brief Validate readout data for non-2D beam monitors
  ///
  /// Performs validation of readout data in the following order:
  /// 1. Ring validation - checks if Ring (calculated from FiberId) is within
  /// [0, MaxRing]
  /// 2. FEN validation - checks if FENId is within [0, MaxFEN]
  /// 3. Topology validation - verifies FEN/Channel combination exists in
  /// configuration
  /// 4. MonitorRing validation - checks if Ring matches configured the Ring
  /// configured for the monitor
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
            CbmBaseStats.MonitorRingMismatchErrors++;
            return false;
          }
          return true;
        },
        // 3. MaxFEN limit validation
        [&]() { return validateFEN(Data.FENId); },
        [&]() {
          return validateTopology(*Conf.TopologyMapPtr, Data.FENId,
                                  Data.Channel);
        });
  }

  ///
  /// \brief Get access to CBM base geometry statistics counters
  ///
  inline const CbmBaseGeometryCounters &getGeometryCounters() const {
    return CbmBaseStats;
  }

  ///
  /// \brief Get the source name for this geometry
  ///
  inline const std::string &getSourceName() const { return SourceName; }

  ///
  /// \brief Get the configured pixel offset
  ///
  inline uint32_t getPixelOffset() const { return PixelOffset; }

protected:
  ///
  /// \brief Implementation of pixel calculation for non-2D geometry
  ///
  /// For EVENT_0D: Returns the fixed pixel offset from topology configuration
  /// For IBM: Returns 0 (pixel not used for histogram data)
  ///
  /// \param Data Const reference to CbmReadout object (unused for fixed pixel)
  /// \return Fixed pixel offset, or 0 for IBM monitors
  ///
  uint32_t calcPixelImpl(const Parser::CbmReadout & /*Data*/) const override {
    return PixelOffset;
  }

private:
  mutable CbmBaseGeometryCounters
      CbmBaseStats;             ///< CBM base statistics counters
  const Config &Conf;           ///< Configuration reference
  const std::string SourceName; ///< Source name for metric prefixing
  const uint32_t PixelOffset;   ///< Fixed pixel value (0 for IBM)
};

} // namespace cbm
