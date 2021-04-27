// Copyright (C) 2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/Assert.h>
#include <common/Trace.h>
#include <logical_geometry/ESSGeometry.h>
#include <stdint.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_WAR

namespace Dream {

class EndCapGeometry {
public:
  const uint8_t MaxSector{22};
  const uint8_t MaxSumo{6};
  const uint8_t MinSumo{3};
  const uint8_t MaxWire{15};
  const uint8_t MaxStrip{15};

  uint32_t getPixel(uint8_t Sector, uint8_t Sumo, uint8_t Cassette,
                    uint8_t Counter, uint8_t Wire, uint8_t Strip) {
    XTRACE(EVENT, DEB, "Sector %u, Sumo %u, Cassette %u, Counter %u, Wire %u, Strip %u",
           Sector, Sumo, Cassette, Counter, Wire, Strip);

    uint16_t X = getX(Sector, Sumo, Cassette, Counter);
    uint16_t Y = getY(Wire, Strip);
    uint32_t Pixel = Geometry.pixel2D(X, Y);

    XTRACE(EVENT, DEB, "x %u, y %u - pixel: %u", X, Y, Pixel);
    return Pixel;
  }

  uint16_t getX(uint8_t Sector, uint8_t Sumo, uint8_t Cassette, uint8_t Counter) {
    if (Sector > MaxSector) {
      XTRACE(EVENT, WAR, "Invalid Sector: %u", Sector);
      return -1;
    }

    if ((Sumo < MinSumo) or (Sumo > MaxSumo)) {
      XTRACE(EVENT, WAR, "Invalid SumoId: %u", Sumo);
      return -1;
    }

    if (Cassette >= SumoCassetteCount[Sumo]) {
      XTRACE(EVENT, WAR, "Invalid CassetteId: %u", Cassette);
      return -1;
    }

    if (Counter > 1) {
      XTRACE(EVENT, WAR, "Invalid Counter: %u", Counter);
      return -1;
    }
    return getXoffset(Sector) + getLocalXCoord(Sumo, Cassette, Counter);
  }

  uint16_t getY(uint8_t Wire, uint8_t Strip) {
    if (Wire > MaxWire) {
      XTRACE(EVENT, WAR, "Invalid Wire: %u", Wire);
      return -1;
    }

    if (Strip > MaxStrip) {
      XTRACE(EVENT, WAR, "Invalid Strip: %u", Strip);
      return -1;
    }
    return getYoffset(Strip) + getLocalYCoord(Wire);
  }

private:
  // these map Sumo Id (3..6) to various SUMO properties.
  const uint8_t SumoOffset[7] = {0, 0, 0, 48, 36, 20, 0};
  const uint8_t SumoCassetteCount[7] = {0, 0, 0, 4, 6, 8, 10};

  uint16_t getLocalXCoord(uint8_t Sumo, uint8_t Cassette, uint8_t Counter) {
    return SumoOffset[Sumo] + 2 * Cassette + Counter;
  }

  uint16_t getLocalYCoord(uint8_t Wire) {
    return Wire;
  }

  uint16_t getXoffset(uint8_t Sector) {
    return Sector * 56;
  }

  uint16_t getYoffset(uint8_t Strip) {
    return Strip * 16;
  }

  ESSGeometry Geometry{1288, 256, 1, 1};
};

} // namespace Dream
