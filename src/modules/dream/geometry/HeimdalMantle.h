// Copyright (C) 2024 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CDT Mantle module abstractions
///
/// Consult ICD for logical geometry dimensions, rotations etc.
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

class HeimdalMantle : public ESSGeometry {
private:
  const uint8_t WiresPerCounter{32};
  const uint16_t StripsPerCass;

public:
  ///\brief Internal method for pixel coordinate calculation
  ///\note: Intended for internal use by calcPixelId and testing only
  int getY(int Strip, int Wire) const { return Strip + Wire * StripsPerCass; }

  ///\brief Internal method for pixel coordinate calculation
  ///\note: Intended for internal use by calcPixelId and testing only
  int getX(int MU, int Cassette, int Counter) const {
    return 12 * MU + 2 * Cassette + Counter;
  }
  ///\brief change default number of strips per cassette
  explicit HeimdalMantle(uint16_t Strips = 64)
      : ESSGeometry{144, 2048, 1, 1}, StripsPerCass(Strips) {}

  //
  uint32_t calcPixelId(const Config::ModuleParms &Parms,
                      const DataParser::CDTReadout &Data) const;
};
} // namespace Dream
