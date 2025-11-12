// Copyright (C) 2023 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from digital identifiers, see latest reviewed
/// ICD for CSPEC:
/// https://project.esss.dk/owncloud/index.php/s/3i0RtWiVwNM6EBY (v1 -- out of
/// date as of 2023-02)
///
//===----------------------------------------------------------------------===//

#include <common/Statistics.h>
#include <logical_geometry/ESSGeometry.h>
#include <modules/cspec/geometry/CspecGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

CspecGeometry::CspecGeometry(Statistics &Stats, const Config &CaenConfiguration)
    : Geometry(Stats, CaenConfiguration.CaenParms.MaxRing,
               CaenConfiguration.CaenParms.MaxFEN,
               CaenConfiguration.CaenParms.MaxGroup),
      ESSGeometry(900, 180, 1, 1),
      Resolution(CaenConfiguration.CaenParms.Resolution) {}

bool CspecGeometry::validateReadoutData(const DataParser::CaenReadout &Data) {
  int Ring = calcRing(Data.FiberId);
  XTRACE(DATA, DEB, "FiberId: %u, Ring %d, FEN %u, Group %u", Data.FiberId,
         Ring, Data.FENId, Data.Group);

  return validateAll([&]() { return validateRing(Ring); },
                     [&]() { return validateFEN(Data.FENId); },
                     [&]() { return validateGroup(Data.Group); });
}

int CspecGeometry::xOffset(int Ring, int Group) const {
  ///\todo Determine the 'real' x-offset once a new ICD is decided for 3He
  /// CSPEC
  // Use per-detector Resolution for ring stride and subdivide for groups.
  return Ring * Resolution + (Group % 24) * (Resolution / 24);
}

int CspecGeometry::posAlongUnit(int AmpA, int AmpB) const {
  if (AmpA + AmpB == 0) {
    ///\todo add counter
    return -1;
  }
  return ((Resolution - 1) * AmpA) / (AmpA + AmpB);
}

uint32_t CspecGeometry::calcPixelImpl(const void *DataPtr) const {
  auto Data = static_cast<const DataParser::CaenReadout *>(DataPtr);
  int Ring = calcRing(Data->FiberId);
  int xoff = xOffset(Ring, Data->Group);
  int ylocal = yCoord(Data->AmpA, Data->AmpB);
  uint32_t pixel = pixel2D(xoff, ylocal);

  XTRACE(DATA, DEB, "xoffset %d, ylocal %d, pixel %hu", xoff, ylocal, pixel);

  return pixel;
}

} // namespace Caen
