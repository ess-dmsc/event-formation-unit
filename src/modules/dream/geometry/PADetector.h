// Copyright (C) 2023 - 2024 European Spallation Source ERIC, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <dream/geometry/Config.h>
#include <dream/readout/DataParser.h>
#include <logical_geometry/ESSGeometry.h>
#include <stdint.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Dream {

class PADetector {
public:
  const uint8_t MaxSector{7};
  const uint8_t MaxWire{15};
  const uint8_t MaxStrip{31};

  const uint8_t CountersPerCass{2};
  const uint8_t StripsPerCass{32};
  const uint8_t WiresPerCounter{16};

  PADetector(uint16_t xdim, uint16_t ydim) : Geometry(xdim, ydim, 1, 1) {}

  ///\brief calculate the cassette id from the digital identifiers:
  /// sumo, anode and cathode.
  /// CDT promises that anodes and cathodes are guaranteed to be consistent
  /// this is necessary because not all combination of the two values are
  /// meaningful.
  /// \todo possibly add some checks here
  uint8_t getCassette(uint8_t Anode, uint8_t Cathode) {
    XTRACE(DATA, DEB, "anode %u, cathode %u", Anode, Cathode);
    return (Anode / 32) + 4 * (Cathode / 64);
  }

  /// \todo CHECK AND VALIDATE, THIS IS UNCONFIRMED
  uint32_t getPixelId(Config::ModuleParms &Parms,
                      DataParser::CDTReadout &Data) {
    uint8_t Sector = Parms.P1.Sector;
    ///\todo two sumos per CDRE or just one?

    uint8_t Cassette = getCassette(Data.Anode, Data.Cathode);

    uint8_t Counter = (Data.Anode / WiresPerCounter) % CountersPerCass;
    uint8_t Wire = Data.Anode % WiresPerCounter;
    uint8_t Strip = Data.Cathode % StripsPerCass;

    XTRACE(EVENT, DEB, "Sector %u, Cassette %u, Counter %u, Wire %u, Strip %u",
           Sector, Cassette, Counter, Wire, Strip);

    uint16_t X = getX(Sector, Cassette, Counter);
    uint16_t Y = getY(Wire, Strip);
    uint32_t Pixel = Geometry.pixel2D(X, Y);

    XTRACE(EVENT, DEB, "x %u, y %u - pixel: %u", X, Y, Pixel);
    return Pixel;
  }

  int getX(uint8_t Sector, uint8_t Cassette, uint8_t Counter) {
    if (Sector > MaxSector) {
      XTRACE(EVENT, WAR, "Invalid Sector: %u", Sector);
      return -1;
    }

    if (Cassette >= SumoCassetteCount) {
      XTRACE(EVENT, WAR, "Invalid CassetteId: %u", Cassette);
      return -1;
    }

    if (Counter > 1) {
      XTRACE(EVENT, WAR, "Invalid Counter: %u", Counter);
      return -1;
    }
    return getXoffset(Sector) + 2 * Cassette + Counter;
  }

  int getY(uint8_t Wire, uint8_t Strip) {
    if (Wire > MaxWire) {
      XTRACE(EVENT, WAR, "Invalid Wire: %u", Wire);
      return -1;
    }

    if (Strip > MaxStrip) {
      XTRACE(EVENT, WAR, "Invalid Strip: %u", Strip);
      return -1;
    }

    int YOffset = getYoffset(Strip);
    int YLocal = getLocalYCoord(Wire);
    XTRACE(EVENT, DEB, "yoffset: %d, localy %d", YOffset, YLocal);
    return YOffset + YLocal;
  }

  ESSGeometry Geometry{1, 1, 1, 1}; // initialised by constructor

private:
  // these map Sumo Id (3..6) to various SUMO properties.
  const uint8_t SumoCassetteCount{16};

  uint16_t getLocalYCoord(uint8_t Wire) { return Wire; }

  uint16_t getXoffset(uint8_t Sector) { return Sector * StripsPerCass; }

  uint16_t getYoffset(uint8_t Strip) { return Strip * WiresPerCounter; }
};

} // namespace Dream
