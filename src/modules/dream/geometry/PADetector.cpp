#include "PADetector.h"
#include <common/debug/Trace.h>

namespace Dream {

uint32_t PADetector::calcPixelId(const Config::ModuleParms &Parms,
                                const DataParser::CDTReadout &Data) const {
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
  uint32_t Pixel = pixel2D(X, Y);

  XTRACE(EVENT, DEB, "x %u, y %u - pixel: %u", X, Y, Pixel);
  return Pixel;
}

int PADetector::getX(uint8_t Sector, uint8_t Cassette, uint8_t Counter) const {
  if (Sector > MaxSector) {
    XTRACE(EVENT, WAR, "Invalid Sector: %u", Sector);
    PADetectorCounters.MaxSectorErrors++;
    return -1;
  }

  if (Cassette >= SumoCassetteCount) {
    XTRACE(EVENT, WAR, "Invalid CassetteId: %u", Cassette);
    PADetectorCounters.CassetteIdErrors++;
    return -1;
  }

  if (Counter > 1) {
    XTRACE(EVENT, WAR, "Invalid Counter: %u", Counter);
    PADetectorCounters.CounterErrors++;
    return -1;
  }
  return getXoffset(Sector) + 2 * Cassette + Counter;
}

int PADetector::getY(uint8_t Wire, uint8_t Strip) const {
  if (Wire > MaxWire) {
    XTRACE(EVENT, WAR, "Invalid Wire: %u", Wire);
    PADetectorCounters.WireErrors++;
    return -1;
  }

  if (Strip > MaxStrip) {
    XTRACE(EVENT, WAR, "Invalid Strip: %u", Strip);
    PADetectorCounters.StripErrors++;
    return -1;
  }

  int YOffset = getYoffset(Strip);
  int YLocal = getLocalYCoord(Wire);
  XTRACE(EVENT, DEB, "yoffset: %d, localy %d", YOffset, YLocal);
  return YOffset + YLocal;
}

} // namespace Dream
