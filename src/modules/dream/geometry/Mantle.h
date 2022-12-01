// Copyright (C) 2022 European Spallation Source, see LICENSE file
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

namespace Dream {

class Mantle {
public:
  int getX(int Strip) { return Strip; }

  /// \brief get global y-coordinate for Cuboid with a given index
  int getY(int MU, int Cassette, int Counter, int Wire) {
    return 120 * Wire + 12 * MU + 2 * Cassette + Counter;
  }

  //
  uint32_t getPixelId(Config::ModuleParms &Parms,
                      DataParser::DreamReadout &Data) {
    /// \todo fix and check all values
    uint8_t MountingUnit = Parms.P1.MU;
    uint8_t Cassette = Parms.P2.Cassette;
    uint8_t Counter = 0; /// \todo part of anode field?
    uint8_t Wire = Data.Cathode;
    uint8_t Strip = Data.Anode;

    int x = getX(Strip);
    int y = getY(MountingUnit, Cassette, Counter, Wire);
    return Geometry.pixel2D(x, y);
  }

private:
  ESSGeometry Geometry{256, 1920, 1, 1};
};
} // namespace Dream
