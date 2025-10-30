// Copyright (C) 2020 - 2025 European Spallation Source, see LICENSE file
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

class SUMO : public ESSGeometry {

private:
  struct SUMOCounters : public StatCounterBase {
    int64_t MaxSectorErrors{0};
    int64_t SumoIdErrors{0};
    int64_t CassetteIdErrors{0};
    int64_t CounterErrors{0};
    int64_t MaxWireErrors{0};
    int64_t MaxStripErrors{0};

    SUMOCounters(Statistics &Stats)
        : StatCounterBase(
              Stats, {
                         {METRIC_COUNTER_MAX_SECTOR_ERRORS, MaxSectorErrors},
                         {METRIC_COUNTER_SUMO_ID_ERRORS, SumoIdErrors},
                         {METRIC_COUNTER_CASSETTE_ID_ERRORS, CassetteIdErrors},
                         {METRIC_COUNTER_COUNTER_ERRORS, CounterErrors},
                         {METRIC_COUNTER_MAX_WIRE_ERRORS, MaxWireErrors},
                         {METRIC_COUNTER_MAX_STRIP_ERRORS, MaxStripErrors},
                     }) {}
  };

  uint16_t getLocalYCoord(uint8_t Wire) const { return Wire; }

  uint16_t getXoffset(uint8_t Sector) const { return Sector * 56; }

  uint16_t getYoffset(uint8_t Strip) const { return Strip * 16; }

  // these map Sumo Id (3..6) to various SUMO properties.
  const uint8_t SumoOffset[7] = {0, 0, 0, 48, 36, 20, 0};
  const uint8_t SumoCassetteCount[7] = {0, 0, 0, 4, 6, 8, 10};

  mutable SUMOCounters SUMOCounters;

public:
  const uint8_t MaxSector{22};
  const uint8_t MaxSumo{6};
  const uint8_t MinSumo{3};
  const uint8_t MaxWire{15};
  const uint8_t MaxStrip{15};

  const uint8_t CountersPerCass{2};
  const uint8_t StripsPerCass{16};
  const uint8_t WiresPerCounter{16};

  // clang-format off
  static inline const std::string METRIC_COUNTER_MAX_SECTOR_ERRORS  = "geometry.sumo.max_sector_errors";
  static inline const std::string METRIC_COUNTER_SUMO_ID_ERRORS     = "geometry.sumo.sumo_id_errors";
  static inline const std::string METRIC_COUNTER_CASSETTE_ID_ERRORS = "geometry.sumo.cassette_id_errors";
  static inline const std::string METRIC_COUNTER_COUNTER_ERRORS     = "geometry.sumo.counter_errors";
  static inline const std::string METRIC_COUNTER_MAX_WIRE_ERRORS    = "geometry.sumo.max_wire_errors";
  static inline const std::string METRIC_COUNTER_MAX_STRIP_ERRORS   = "geometry.sumo.max_strip_errors";
  // clang-format on

  SUMO(Statistics &Stats, uint16_t xdim, uint16_t ydim)
      : ESSGeometry(xdim, ydim, 1, 1), SUMOCounters(Stats) {}

  /// \brief Get access to SUMO-specific geometry statistics counters
  inline const struct SUMOCounters &getSUMOCounters() const {
    return SUMOCounters;
  }

  ///\brief calculate the cassette id from the digital identifiers:
  /// sumo, anode and cathode.
  /// CDT promises that anodes and cathodes are guaranteed to be consistent
  /// this is necessary because not all combination of the two values are
  /// meaningful.
  /// \note: this function intended to be used only internally by calcPixelId
  ///       or testing
  /// \todo possibly add some checks here
  uint8_t getCassette(uint8_t Sumo, uint8_t Anode, uint8_t Cathode) const;

  /// \todo CHECK AND VALIDATE, THIS IS UNCONFIRMED
  uint32_t calcPixelId(const Config::ModuleParms &Parms,
                      const DataParser::CDTReadout &Data) const;

  ///\brief Internal method for pixel coordinate calculation
  ///\note: Intended for internal use by calcPixelId and testing only
  int getX(uint8_t Sector, uint8_t Sumo, uint8_t Cassette,
           uint8_t Counter) const;

  ///\brief Internal method for pixel coordinate calculation
  ///\note: Intended for internal use by calcPixelId and testing only
  int getY(uint8_t Wire, uint8_t Strip) const;
};

} // namespace Dream
