// Copyright (C) 2020 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <cstdint>
#include <dream/geometry/SUMO.h>

namespace Dream {

uint8_t SUMO::getCassette(uint8_t Sumo, uint8_t Anode, uint8_t Cathode) const {
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

uint32_t SUMO::calcPixelId(const Config::ModuleParms &Parms,
                          const DataParser::CDTReadout &Data) const {
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
  uint32_t Pixel = pixel2D(X, Y);

  XTRACE(EVENT, DEB, "x %u, y %u - pixel: %u", X, Y, Pixel);
  return Pixel;
}

int SUMO::getX(uint8_t Sector, uint8_t Sumo, uint8_t Cassette,
               uint8_t Counter) const {
  if (Sector > MaxSector) {
    XTRACE(EVENT, WAR, "Invalid Sector: %u", Sector);
    SUMOCounters.MaxSectorErrors++;
    return -1;
  }

  if ((Sumo < MinSumo) or (Sumo > MaxSumo)) {
    XTRACE(EVENT, WAR, "Invalid SumoId: %u", Sumo);
    SUMOCounters.SumoIdErrors++;
    return -1;
  }

  if (Cassette >= SumoCassetteCount[Sumo]) {
    XTRACE(EVENT, WAR, "Invalid CassetteId: %u", Cassette);
    SUMOCounters.CassetteIdErrors++;
    return -1;
  }

  if (Counter > 1) {
    XTRACE(EVENT, WAR, "Invalid Counter: %u", Counter);
    SUMOCounters.CounterErrors++;
    return -1;
  }
  return getXoffset(Sector) + SumoOffset[Sumo] + 2 * Cassette + Counter;
}

int SUMO::getY(uint8_t Wire, uint8_t Strip) const {
  if (Wire > MaxWire) {
    XTRACE(EVENT, WAR, "Invalid Wire: %u", Wire);
    SUMOCounters.MaxWireErrors++;
    return -1;
  }

  if (Strip > MaxStrip) {
    XTRACE(EVENT, WAR, "Invalid Strip: %u", Strip);
    SUMOCounters.MaxStripErrors++;
    return -1;
  }
  return getYoffset(Strip) + getLocalYCoord(Wire);
}

} // namespace Dream
