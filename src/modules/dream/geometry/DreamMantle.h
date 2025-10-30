// Copyright (C) 2022 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CDT Mantle module abstractions
///
/// Consult ICD for logical geometry dimensions, rotations etc.
/// The Dream mantle is also used by Magic.
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

class DreamMantle : public ESSGeometry {
private:
  uint16_t StripsPerCass{256}; // for DREAM, 128 for Magic
  const uint8_t WiresPerCounter{32};

public:
  ///\brief Internal method for pixel coordinate calculation
  ///\note: Intended for internal use by calcPixelId and testing only
  int getX(int Strip) const { return Strip; }

  ///\brief Internal method for pixel coordinate calculation
  ///\note: Intended for internal use by calcPixelId and testing only
  int getY(int MU, int Cassette, int Counter, int Wire) const {
    return 60 * Wire + 12 * MU + 2 * Cassette + Counter;
  }

  ///\brief change default number of strips per cassette to differentiate
  /// between DREAM (256) and MAGIC (128)
  explicit DreamMantle(Statistics &, uint16_t Strips)
      : ESSGeometry(Strips, 1920, 1, 1), StripsPerCass(Strips) {}

  //
  uint32_t calcPixelId(const Config::ModuleParms &Parms,
                      const DataParser::CDTReadout &Data) const;
};
} // namespace Dream
