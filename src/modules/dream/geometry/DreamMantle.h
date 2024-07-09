// Copyright (C) 2022 - 2024 European Spallation Source, see LICENSE file
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

#include <common/debug/Trace.h>
#include <dream/geometry/Config.h>
#include <dream/readout/DataParser.h>
#include <logical_geometry/ESSGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Dream {

class DreamMantle {
public:

  ///\brief change default number of strips per cassette to differentiate
  /// between DREAM (256) and MAGIC (128)
  explicit DreamMantle(uint16_t Strips) : StripsPerCass(Strips){};

  int getX(int Strip) { return Strip; }

  /// \brief get global y-coordinate for Cuboid with a given index
  int getY(int MU, int Cassette, int Counter, int Wire) {
    return 60 * Wire + 12 * MU + 2 * Cassette + Counter;
  }

  //
  uint32_t getPixelId(Config::ModuleParms &Parms,
                      DataParser::DreamReadout &Data) {
    uint8_t MountingUnit = Parms.P1.MU;
    uint8_t Cassette = Parms.P2.Cassette;
    uint8_t Counter = (Data.Anode / WiresPerCounter) % 2;
    uint8_t Wire = Data.Anode % WiresPerCounter;
    uint8_t Strip = Data.Cathode % StripsPerCass;

    XTRACE(DATA, DEB, "M.U. %u, Cassette %u, Counter %u, WIre %u, Strip %u",
           MountingUnit, Cassette, Counter, Wire, Strip);

    int x = getX(Strip);
    int y = getY(MountingUnit, Cassette, Counter, Wire);
    return Geometry.pixel2D(x, y);
  }

private:
  uint16_t StripsPerCass{256}; // for DREAM, 128 for Magic
  const uint8_t WiresPerCounter{32};
  ESSGeometry Geometry{256, 1920, 1, 1};
};
} // namespace Dream
