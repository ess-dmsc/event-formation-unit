// Copyright (C) 2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CDT Mantle module abstractions
///
/// Consult ICD for logical geometry dimensions, rotations etc.
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <dream/geometry/Config.h>
#include <dream/readout/DataParser.h>
#include <logical_geometry/ESSGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Dream {

class HeimdalMantle {
public:
  ///\brief change default number of strips per cassette
  explicit HeimdalMantle(uint16_t Strips) : StripsPerCass(Strips){};

  int getY(int Strip, int Wire) const { return Strip + Wire * StripsPerCass; }

  /// \brief get global y-coordinate for Cuboid with a given index
  int getX(int MU, int Cassette, int Counter) const {
    return 12 * MU + 2 * Cassette + Counter;
  }

  //
  uint32_t getPixelId(Config::ModuleParms &Parms,
                      DataParser::CDTReadout &Data) const {
    uint8_t MountingUnit = Parms.P1.MU;
    uint8_t Cassette = Parms.P2.Cassette;
    uint8_t Counter = (Data.Anode / WiresPerCounter) % 2;
    uint8_t Wire = Data.Anode % WiresPerCounter;
    uint8_t Strip = Data.Cathode % StripsPerCass;

    XTRACE(DATA, DEB, "M.U. %u, Cassette %u, Counter %u, Wire %u, Strip %u",
           MountingUnit, Cassette, Counter, Wire, Strip);

    int y = getY(Strip, Wire);
    int x = getX(MountingUnit, Cassette, Counter);
    return Geometry.pixel2D(x, y);
  }

private:
  const uint8_t WiresPerCounter{32};
  const uint16_t StripsPerCass{64};
  ESSGeometry Geometry{144, 2048, 1, 1};
};
} // namespace Dream
