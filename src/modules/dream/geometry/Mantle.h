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
  const uint8_t WiresPerCounter{32};
  uint16_t StripsPerCass{256};

  ///\brief change default number of strips per cassette to differentiate
  /// between DREAM (256) and MAGIC (128)
  explicit Mantle(uint16_t Strips) : StripsPerCass(Strips) {};

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

    int x = getX(Strip);
    int y = getY(MountingUnit, Cassette, Counter, Wire);
    return Geometry.pixel2D(x, y);
  }

private:
  ESSGeometry Geometry{256, 1920, 1, 1};
};
} // namespace Dream
