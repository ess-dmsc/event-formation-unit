// Copyright (C) 2023 - 2025 European Spallation Source ERIC, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/Statistics.h>
#include <common/debug/Trace.h>
#include <cstdint>
#include <dream/geometry/Config.h>
#include <dream/readout/DataParser.h>
#include <logical_geometry/ESSGeometry.h>
#include <stdint.h>
#include <string>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Dream {

class PADetector : public ESSGeometry {
private:
  struct PADetectorCounters : public StatCounterBase {
    int64_t MaxSectorErrors{0};
    int64_t CassetteIdErrors{0};
    int64_t CounterErrors{0};
    int64_t WireErrors{0};
    int64_t StripErrors{0};

    PADetectorCounters(Statistics &Stats)
        : StatCounterBase(
              Stats, {
                         {METRIC_COUNTER_MAX_SECTOR_ERRORS, MaxSectorErrors},
                         {METRIC_COUNTER_CASSETTE_ID_ERRORS, CassetteIdErrors},
                         {METRIC_COUNTER_COUNTER_ERRORS, CounterErrors},
                         {METRIC_COUNTER_WIRE_ERRORS, WireErrors},
                         {METRIC_COUNTER_STRIP_ERRORS, StripErrors},
                     }) {}
  };

  ///\brief calculate the cassette id from the digital identifiers:
  /// sumo, anode and cathode.
  /// CDT promises that anodes and cathodes are guaranteed to be consistent
  /// this is necessary because not all combination of the two values are
  /// meaningful.
  /// \todo possibly add some checks here
  uint8_t getCassette(uint8_t Anode, uint8_t Cathode) const {
    XTRACE(DATA, DEB, "anode %u, cathode %u", Anode, Cathode);
    return (Anode / 32) + 4 * (Cathode / 64);
  }

  uint16_t getLocalYCoord(uint8_t Wire) const { return Wire; }

  uint16_t getXoffset(uint8_t Sector) const { return Sector * StripsPerCass; }

  uint16_t getYoffset(uint8_t Strip) const { return Strip * WiresPerCounter; }

  mutable PADetectorCounters PADetectorCounters;
  // these map Sumo Id (3..6) to various SUMO properties.
  const uint8_t SumoCassetteCount{16};

public:
  // clang-format off
  static inline const std::string METRIC_COUNTER_MAX_SECTOR_ERRORS  = "geometry.padetector.max_sector_errors";
  static inline const std::string METRIC_COUNTER_CASSETTE_ID_ERRORS = "geometry.padetector.cassette_id_errors";
  static inline const std::string METRIC_COUNTER_COUNTER_ERRORS     = "geometry.padetector.counter_errors";
  static inline const std::string METRIC_COUNTER_WIRE_ERRORS        = "geometry.padetector.wire_errors";
  static inline const std::string METRIC_COUNTER_STRIP_ERRORS       = "g eometry.padetector.strip_errors";
  // clang-format on

  const uint8_t MaxSector{7};
  const uint8_t MaxWire{15};
  const uint8_t MaxStrip{31};

  const uint8_t CountersPerCass{2};
  const uint8_t StripsPerCass{32};
  const uint8_t WiresPerCounter{16};

  PADetector(Statistics &Stats, uint16_t xdim, uint16_t ydim)
      : ESSGeometry(xdim, ydim, 1, 1), PADetectorCounters(Stats) {}

  /// \todo CHECK AND VALIDATE, THIS IS UNCONFIRMED
  uint32_t calcPixelId(const Config::ModuleParms &Parms,
                      const DataParser::CDTReadout &Data) const;

  const struct PADetectorCounters &getPADetectorCounters() const {
    return PADetectorCounters;
  }

  ///\brief Internal method for pixel coordinate calculation
  ///\note: Intended for internal use by calcPixelId and testing only
  int getX(uint8_t Sector, uint8_t Cassette, uint8_t Counter) const;

  ///\brief Internal method for pixel coordinate calculation
  ///\note: Intended for internal use by calcPixelId and testing only
  int getY(uint8_t Wire, uint8_t Strip) const;
};

} // namespace Dream
