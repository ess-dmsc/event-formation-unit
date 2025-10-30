// Copyright (C) 2022 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CDT Mantle module abstractions
///
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <cstdint>
#include <dream/geometry/DreamMantle.h>

namespace Dream {

uint32_t DreamMantle::calcPixelId(const Config::ModuleParms &Parms,
                                 const DataParser::CDTReadout &Data) const {
  uint8_t MountingUnit = Parms.P1.MU;
  uint8_t Cassette = Parms.P2.Cassette;
  uint8_t Counter = (Data.Anode / WiresPerCounter) % 2;
  uint8_t Wire = Data.Anode % WiresPerCounter;
  uint8_t Strip = Data.Cathode % StripsPerCass;

  XTRACE(DATA, DEB, "M.U. %u, Cassette %u, Counter %u, Wire %u, Strip %u",
         MountingUnit, Cassette, Counter, Wire, Strip);

  int x = getX(Strip);
  int y = getY(MountingUnit, Cassette, Counter, Wire);
  return pixel2D(x, y);
}

} // namespace Dream
