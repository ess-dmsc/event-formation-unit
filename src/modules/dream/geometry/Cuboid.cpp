#include "Cuboid.h"
#include <common/debug/Trace.h>

namespace Dream {

void Cuboid::rotateXY(int Rotate, int &LocalX, int &LocalY) const {
  int SavedY = LocalY;

  switch (Rotate) {
  case 1: // 90 deg. clockwise
    LocalY = LocalX;
    LocalX = 15 - SavedY;
    break;
  case 2: // 180 deg. cockwise
    LocalY = 15 - LocalY;
    LocalX = 15 - LocalX;
    break;
  case 3: // 270 deg. clockwise
    LocalY = 15 - LocalX;
    LocalX = SavedY;
    break;
  }
}

uint32_t Cuboid::calcPixelId(const Config::ModuleParms &Parms,
                             const DataParser::CDTReadout &Data) const {
  uint8_t Index = Parms.P1.Index;
  Index += Data.UnitId;

  XTRACE(DATA, DEB, "index %u, anode %u, cathode %u", Index, Data.Anode,
         Data.Cathode);
  uint8_t Cassette = Data.Anode / 32 + 2 * (Data.Cathode / 32);
  uint8_t Counter = (Data.Anode / WIRES_PER_COUNTER) % 2;
  uint8_t Wire = Data.Anode % WIRES_PER_COUNTER;
  uint8_t Strip = Data.Cathode % STRIPS_PER_CASSETTE;

  XTRACE(DATA, DEB, "cass %u, ctr %u, wire %u, strip %u", Cassette, Counter,
         Wire, Strip);

  CuboidOffset Offset;
  int Rotation;
  if (Parms.Type == Config::ModuleType::SANS) {
    if (Index >= (int)OffsetsSANS.size()) {
      XTRACE(DATA, WAR, "Bad SANS index %u", Index);
      CuboidCounters.IndexErrors++;
      return 0;
    }
    Offset = OffsetsSANS[Index];
    Rotation = RotateSANS[Index];
  } else if (Parms.Type == Config::ModuleType::HR) {
    if (Index >= (int)OffsetsHR.size()) {
      XTRACE(DATA, WAR, "Bad HR index %u", Index);
      CuboidCounters.IndexErrors++;
      return 0;
    }
    Offset = OffsetsHR[Index];
    Rotation = RotateHR[Index];
  } else {
    XTRACE(DATA, WAR, "Inconsistent type (%d) for Cuboid", Parms.Type);
    CuboidCounters.TypeErrors++;
    return 0;
  }

  int LocalX = 2 * Cassette + Counter; // unrotated x,y values
  int LocalY = 15 - Wire;

  XTRACE(DATA, DEB, "local x %u, local y %u, rotate %u", LocalX, LocalY,
         Rotation);

  rotateXY(Rotation, LocalX, LocalY);

  constexpr int YDim{7 * 16};
  int IntX = Offset.X + LocalX;
  int IntY = YDim * Strip + Offset.Y + LocalY;
  XTRACE(DATA, DEB, "x %d, y %d", IntX, IntY);

  // Validate coordinates before casting to prevent overflow
  if (IntX < 0 || IntX > UINT16_MAX) {
    XTRACE(DATA, WAR, "x coordinate out of range: %d", IntX);
    return 0; // Invalid pixel
  }
  if (IntY < 0 || IntY > UINT16_MAX) {
    XTRACE(DATA, WAR, "y coordinate out of range: %d", IntY);
    return 0; // Invalid pixel
  }

  return pixel2D(static_cast<uint16_t>(IntX), static_cast<uint16_t>(IntY));
}

} // namespace Dream
