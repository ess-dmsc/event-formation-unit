// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from digital identifiers, see latest reviewed
/// ICD for Tbl3He:
/// https://project.esss.dk/nextcloud/index.php/s/3SM88TjNfKxyPrz
///
//===----------------------------------------------------------------------===//

#include <modules/tbl3he/geometry/Tbl3HeGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

Tbl3HeGeometry::Tbl3HeGeometry(Config &CaenConfiguration) {
  ESSGeom = new ESSGeometry(100, 8, 1, 1);
  setResolution(CaenConfiguration.Resolution);
  MaxRing = CaenConfiguration.MaxRing;
  MaxFEN = CaenConfiguration.MaxFEN;
  MaxGroup = CaenConfiguration.MaxGroup;
}

bool Tbl3HeGeometry::validateData(DataParser::CaenReadout &Data) {
  int Ring = Data.FiberId / 2;
  XTRACE(DATA, DEB, "Fiber %u, Ring %d, FEN %u, Group %u", Data.FiberId, Ring,
         Data.FENId, Data.Group);

  if (Ring > MaxRing) {
    XTRACE(DATA, WAR, "RING %d is incompatible with config", Ring);
    Stats.RingErrors++;
    return false;
  }

  if (Data.FENId > MaxFEN) {
    XTRACE(DATA, WAR, "FEN %d is incompatible with config", Data.FENId);
    Stats.FENErrors++;
    return false;
  }

  if (Data.Group > MaxGroup) {
    XTRACE(DATA, WAR, "Group %d is incompatible with config", Data.Group);
    Stats.GroupErrors++;
    return false;
  }
  return true;
}


/// \brief calulate the pixel id from the readout data
/// \return 0 for invalid pixel, nonzero for good pixels
uint32_t Tbl3HeGeometry::calcPixel(DataParser::CaenReadout __attribute__((unused)) &Data) {
  int Ring = Data.FiberId / 2;
  int Tube = Data.Group;
  uint16_t YCoord = (uint32_t)(Ring * 4 + Tube);

  int Denominator = Data.AmpA + Data.AmpB;
  if (Denominator == 0) {
    XTRACE(DATA, WAR, "Sum of amplitudes is zero");
    Stats.AmplitudeZero++;
    return 0;
  }
  uint16_t XCoord = (UnitPixellation - 1)*Data.AmpA/Denominator;
  int PixelId = YCoord * UnitPixellation + XCoord + 1;
  if (PixelId > 8 * UnitPixellation) {
    return 0;
  }
  XTRACE(DATA, DEB, "Ring %d, FEN %d, Tube %d, A %d, B %d, Pixel %d", Ring,
      Data.FENId, Tube, Data.AmpA, Data.AmpB, PixelId);
  return PixelId;
}

} // namespace Caen
