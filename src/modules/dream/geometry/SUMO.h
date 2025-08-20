// Copyright (C) 2020 - 2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <cstdint>
#include <dream/geometry/Config.h>
#include <dream/readout/DataParser.h>
#include <logical_geometry/ESSGeometry.h>
#include <stdint.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Dream {

class SUMO {
public:
  const uint8_t MaxSector{22};
  const uint8_t MaxSumo{6};
  const uint8_t MinSumo{3};
  const uint8_t MaxWire{15};
  const uint8_t MaxStrip{15};

  const uint8_t CountersPerCass{2};
  const uint8_t StripsPerCass{16};
  const uint8_t WiresPerCounter{16};

  SUMO(uint16_t xdim, uint16_t ydim) : Geometry(xdim, ydim, 1, 1) {}

  ///\brief calculate the cassette id from the digital identifiers:
  /// sumo, anode and cathode.
  /// CDT promises that anodes and cathodes are guaranteed to be consistent
  /// this is necessary because not all combination of the two values are
  /// meaningful.
  /// \todo possibly add some checks here
  uint8_t getCassette(uint8_t Sumo, uint8_t Anode, uint8_t Cathode) {
    XTRACE(DATA, DEB, "sumo %u, anode %u, cathode %u", Sumo, Anode, Cathode);
    switch (Sumo) {
    case 6:
      return -(Anode / 32) + 2 * (Cathode / 16);
      break;
    case 5:
      return (Anode / 32) + 2 * (Cathode / 16);
      break;
    case 4:
      return (Anode / 32) + 2 * (Cathode / 16) - 1;
      break;
    case 3:
      return (Anode / 32) + 2 * (Cathode / 32);
      break;
    default:
      return 255;
      break;
    }
  }

  /// \todo CHECK AND VALIDATE, THIS IS UNCONFIRMED
  uint32_t getPixelId(Config::ModuleParms &Parms,
                      DataParser::CDTReadout &Data) {
    uint8_t Sector = Parms.P1.Sector;
    ///\todo sumo should be identified by the 'Unused' field
    /// and sanity checked with config
    /// config of SumoPair could be an encoding like:
    /// high four nibbles = instance 0 type
    /// low four nibbles - instance 1 type
    /// \todo below is still wrong as we do not mask out the
    /// instance value and the type field fom 'Unused'
    /// But it is less wrong than before ;-)
    uint8_t Sumo = Data.UnitId;

    uint8_t Cassette = getCassette(Sumo, Data.Anode, Data.Cathode);

    uint8_t Counter = (Data.Anode / WiresPerCounter) % CountersPerCass;
    uint8_t Wire = Data.Anode % WiresPerCounter;
    uint8_t Strip = Data.Cathode % StripsPerCass;

    XTRACE(EVENT, DEB,
           "Sector %u, Sumo %u, Cassette %u, Counter %u, Wire %u, Strip %u",
           Sector, Sumo, Cassette, Counter, Wire, Strip);

    int IntX = getX(Sector, Sumo, Cassette, Counter);
    int IntY = getY(Wire, Strip);
    if (IntX < 0 || IntX > UINT16_MAX) {
      XTRACE(EVENT, WAR, "x_val out of uint16_t range: %d", IntX);
      // PixelId 0 is invalid
      return 0;
    }
    if (IntY < 0 || IntY > UINT16_MAX) {
      XTRACE(EVENT, WAR, "y_val out of uint16_t range: %d", IntY);
      // PixelId 0 is invalid
      return 0;
    }
    uint16_t X = static_cast<uint16_t>(IntX);
    uint16_t Y = static_cast<uint16_t>(IntY);
    uint32_t Pixel = Geometry.pixel2D(X, Y);

    XTRACE(EVENT, DEB, "x %u, y %u - pixel: %u", X, Y, Pixel);
    return Pixel;
  }

  int getX(uint8_t Sector, uint8_t Sumo, uint8_t Cassette, uint8_t Counter) {
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
    return getXoffset(Sector) + SumoOffset[Sumo] + 2 * Cassette + Counter;
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
    return getYoffset(Strip) + getLocalYCoord(Wire);
  }

  ESSGeometry Geometry{1, 1, 1, 1}; // initialised by constructor

private:
  // these map Sumo Id (3..6) to various SUMO properties.
  const uint8_t SumoOffset[7] = {0, 0, 0, 48, 36, 20, 0};
  const uint8_t SumoCassetteCount[7] = {0, 0, 0, 4, 6, 8, 10};

  uint16_t getLocalYCoord(uint8_t Wire) { return Wire; }

  uint16_t getXoffset(uint8_t Sector) { return Sector * 56; }

  uint16_t getYoffset(uint8_t Strip) { return Strip * 16; }
};

} // namespace Dream
