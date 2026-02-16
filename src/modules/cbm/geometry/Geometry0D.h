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
/// \brief geometry class for CBM non-2D beam monitors (EVENT_0D and IBM)
///
/// This class provides geometry handling and validation for EVENT_0D and IBM
/// type beam monitors. These monitor types don't have 2D position data but
/// still require Ring/FEN/Topology validation based on fiber cabling.
///
/// For EVENT_0D: Returns a fixed pixel offset from the topology configuration
/// For IBM: Returns 0 (pixel not used, histogram uses ADC values instead)
///
class Geometry0D : public Geometry {
public:

  ///
  /// \brief Constructor
  ///
  /// \param Stats Reference to Statistics object for counter registration
  /// \param Conf Reference to CBM configuration (MaxRing and MaxFENId are read
  /// from here) 
  /// \param MonitorType Beam monitor sub type, IBM, EVENT_0D, EVENT_2D
  /// \param SourceName Name of this geometry (e.g., "cbm1") for
  /// metricprefixing 
  /// \param PixelOffset Fixed pixel value to return for
  /// EVENT_0D (0 for IBM)
  ///
  Geometry0D(Statistics &Stats, const Config &Conf, const CbmType &MonitorType,
             const std::string &SourceName, uint32_t PixelOffset = 0)
      : Geometry(Stats, MonitorType, SourceName, 
          Conf.CbmParms.MaxRing, Conf.CbmParms.MaxFENId, Conf.CbmParms.MonitorRing),
        Conf(Conf), PixelOffset(PixelOffset) {}

  ///
  /// \brief Validate readout data for non-2D beam monitors
  ///
  /// Performs validation of readout data in the following order:
  /// 1. Type validation - checks if beam monitor type is correct
  /// 2. Ring validation - checks if Ring (calculated from FiberId) is within
  /// [0, MaxRing]
  /// 3. FEN validation - checks if FENId is within [0, MaxFEN]
  /// 4. Topology validation - verifies FEN/Channel combination exists in
  /// configuration
  /// 5. MonitorRing validation - checks if Ring matches configured the Ring
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
        // 1. CBM Type Validation
        [&]() { return validateMonitorType(Data.Type); },
        // 2. MaxRing limit validation
        [&]() { return validateRing(Ring); },
        // 3. MonitorRing validation
        [&]() { return validateMonitorRing(Ring); },
        // 4. MaxFEN limit validation
        [&]() { return validateFEN(Data.FENId); },
        // 5. MonitorRing validation
        [&]() {
          return validateTopology(*Conf.TopologyMapPtr, Data.FENId,
                                  Data.Channel);
        });
  }

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
  const Config &Conf;           ///< Configuration reference
  const uint32_t PixelOffset;   ///< Fixed pixel value (0 for IBM)
};

} // namespace cbm
