// Copyright (C) 2024 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CDT Mantle module abstractions
///
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <cstdint>
#include <dream/geometry/HeimdalMantle.h>

namespace Dream {

uint32_t HeimdalMantle::calcPixelId(const Config::ModuleParms &Parms,
                                   const DataParser::CDTReadout &Data) const {
  uint8_t MountingUnit = Parms.P1.MU;
  uint8_t Cassette = Parms.P2.Cassette;
  uint8_t Counter = (Data.Anode / WiresPerCounter) % 2;
  uint8_t Wire = Data.Anode % WiresPerCounter;
  uint8_t Strip = Data.Cathode % StripsPerCass;

  XTRACE(DATA, DEB, "M.U. %u, Cassette %u, Counter %u, Wire %u, Strip %u",
         MountingUnit, Cassette, Counter, Wire, Strip);

  const int y = getY(Strip, Wire);
  const int x = getX(MountingUnit, Cassette, Counter);
  return pixel2D(x, y);
}

} // namespace Dream
