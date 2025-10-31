// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from digital identifiers, see latest reviewed
/// ICD for Miracles:
/// https://project.esss.dk/owncloud/index.php/s/AMKp67jcTGmCFmt
///
//===----------------------------------------------------------------------===//

#include <cmath>
#include <common/Statistics.h>
#include <logical_geometry/ESSGeometry.h>
#include <modules/miracles/geometry/MiraclesGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

MiraclesGeometry::MiraclesGeometry(Statistics &Stats,
                                   const Config &CaenConfiguration, int MaxAmpl)
    : Geometry(Stats, CaenConfiguration.CaenParms.MaxRing,
               CaenConfiguration.CaenParms.MaxFEN, 1, MaxAmpl),
      ESSGeometry(48, 128, 1, 1),
      GroupResolution(CaenConfiguration.CaenParms.Resolution) {}

uint32_t MiraclesGeometry::calcPixelImpl(const void *DataPtr) const {
  auto Data = static_cast<const DataParser::CaenReadout *>(DataPtr);

  int Ring = Data->FiberId / 2;
  int x = xCoord(Ring, Data->Group, Data->AmpA, Data->AmpB);
  int y = yCoord(Ring, Data->AmpA, Data->AmpB);
  uint32_t pixel = pixel2D(x, y);

  XTRACE(DATA, DEB, "xcoord %d, ycoord %d, pixel %hu", x, y, pixel);

  return pixel;
}

bool MiraclesGeometry::validateReadoutData(
    const DataParser::CaenReadout &Data) {
  int Ring = Data.FiberId / 2;
  XTRACE(DATA, DEB, "Ring %u, FEN %u, Group %u", Ring, Data.FENId, Data.Group);

  return validateAll(
      [&]() { return validateRing(Ring); },
      [&]() { return validateAmplitudeZero(Data.AmpA, Data.AmpB); },
      [&]() { return validateAmplitudeHigh(Data.AmpA, Data.AmpB); });
}

int MiraclesGeometry::xCoord(int Ring, int Tube, int AmpA, int AmpB) const {
  int xOffset = 2 * Tube;
  if ((Ring == 1) or (Ring == 3)) {
    xOffset += 24; ///\todo make config dependent
  }
  return xOffset + tubeAorB(AmpA, AmpB);
}

int MiraclesGeometry::yCoord(int Ring, int AmpA, int AmpB) const {
  XTRACE(DATA, DEB, "Calculating yCoord, Ring: %u, AmpA: %u, AmpB: %u", Ring,
         AmpA, AmpB);
  int offset{0};
  if ((Ring == 2) or (Ring == 3)) {
    offset += GroupResolution / 2;
  }
  return offset + posAlongUnit(AmpA, AmpB);
}

int MiraclesGeometry::posAlongUnit(int AmpA, int AmpB) const {
  int tubepos;
  if (AmpA + AmpB == 0) {
    XTRACE(DATA, WAR, "AmpA + AmpB == 0, invalid amplitudes");
    ///\todo add counter
    return -1;
  }

  float pos = (1.0 * AmpA / (AmpA + AmpB));
  XTRACE(DATA, DEB, "Position along tube pair %f", pos);

  if (tubeAorB(AmpA, AmpB) == 0) {
    tubepos =
        round(static_cast<int>(GroupResolution / 2) - 1 -
              (pos - 0.5) * 2 * (static_cast<int>(GroupResolution / 2 - 1)));
    XTRACE(DATA, DEB, "A: TubePos %u, pos: %f", tubepos, pos);
    return tubepos;
  } else {
    tubepos = pos * 2 * (static_cast<int>(GroupResolution / 2) - 1);
    XTRACE(DATA, DEB, "B: TubePos %u, pos: %f", tubepos, pos);
    return tubepos;
  }
}

} // namespace Caen
