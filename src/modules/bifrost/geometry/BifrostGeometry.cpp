// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from digital identifiers, see latest reviewed
/// ICD for Bifrost:
/// https://project.esss.dk/owncloud/index.php/s/AMKp67jcTGmCFmt
///
//===----------------------------------------------------------------------===//

#include <logical_geometry/ESSGeometry.h>
#include <modules/bifrost/geometry/BifrostGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

BifrostGeometry::BifrostGeometry(Statistics &Stats, Config &CaenConfiguration)
    : Geometry(Stats, CaenConfiguration.CaenParms.MaxRing,
               CaenConfiguration.CaenParms.MaxFEN,
               CaenConfiguration.CaenParms.MaxGroup,
               CaenConfiguration.BifrostConf.Parms.MaxAmpl),
      ESSGeometry(900, 15, 1, 1),
      StrideResolution(CaenConfiguration.CaenParms.Resolution),
      Conf(CaenConfiguration) {}

bool BifrostGeometry::validateReadoutData(const DataParser::CaenReadout &Data) {
  int Ring = Data.FiberId / 2;
  XTRACE(DATA, DEB, "Fiber %u, Ring %d, FEN %u, Group %u", Data.FiberId, Ring,
         Data.FENId, Data.Group);

  return validateAll([&]() { return validateRing(Ring); },
                     [&]() { return validateFEN(Data.FENId); },
                     [&]() { return validateGroup(Data.Group); });
}

int BifrostGeometry::xOffset(int Ring, int Group) {
  int RingOffset = Ring * StrideResolution;
  int GroupOffset = (Group % 3) * UnitPixellation;
  XTRACE(DATA, DEB, "RingOffset %d, GroupOffset %d", RingOffset, GroupOffset);
  return RingOffset + GroupOffset;
}

int BifrostGeometry::yOffset(int Group) {
  int Arc = Group / 3; // 3 == triplets per arc (for a given ring)
  return Arc * UnitsPerGroup;
}

std::pair<int, double> BifrostGeometry::calcUnitAndPos(int Group, int AmpA,
                                                       int AmpB) {

  if (AmpA + AmpB == 0) {
    XTRACE(DATA, DEB, "Sum of amplitudes is 0");
    CaenStats.AmplitudeZero++;
    return InvalidPos;
  }

  if (AmpA + AmpB > MaxAmpl) {
    XTRACE(DATA, DEB, "Sum of amplitudes exceeds maximum");
    CaenStats.AmplitudeHigh++;
    return InvalidPos;
  }

  double GlobalPos = 1.0 * AmpA / (AmpA + AmpB); // [0.0 ; 1.0]
  if ((GlobalPos < 0) or (GlobalPos > 1.0)) {
    XTRACE(DATA, WAR, "Pos %f not in unit interval", GlobalPos);
    return InvalidPos;
  }

  int Unit = CaenCDCalibration.getUnitId(Group, GlobalPos);
  if (Unit == -1) {
    XTRACE(DATA, DEB, "A %d, B %d, GlobalPos %f outside valid region", AmpA,
           AmpB, GlobalPos);
    return InvalidPos;
  }

  ///\brief raw unit pos will be in the interval [0;1] regardless of the width
  /// of the interval
  auto &Intervals = CaenCDCalibration.Intervals[Group];
  double Lower = Intervals[Unit].first;
  double Upper = Intervals[Unit].second;
  double RawUnitPos = (GlobalPos - Lower) / (Upper - Lower);

  XTRACE(DATA, DEB, "Unit %d, GlobalPos %f, RawUnitPos %f", Unit, GlobalPos,
         RawUnitPos);
  return std::make_pair(Unit, RawUnitPos);
}

uint32_t BifrostGeometry::calcPixelImpl(const void *DataPtr) {
  auto Data = static_cast<const DataParser::CaenReadout *>(DataPtr);
  int Ring = Data->FiberId / 2;
  int xoff = xOffset(Ring, Data->Group);
  int yoff = yOffset(Data->Group);

  int Group = Ring * TripletsPerRing + Data->Group;
  std::pair<int, double> UnitPos =
      calcUnitAndPos(Group, Data->AmpA, Data->AmpB);

  if (UnitPos.first == -1) {
    return 0;
  }

  int ylocal = UnitPos.first;
  int xlocal = UnitPos.second * (UnitPixellation - 1);
  int X = xoff + xlocal;
  int Y = yoff + ylocal;

  uint32_t pixel = pixel2D(X, Y);
  if (pixel == 0) {
    XTRACE(DATA, WAR, "xoffset %d, xlocal %d, yoffset %d, ylocal %d, pixel %hu",
           xoff, xlocal, yoff, ylocal, pixel);
  } else {
    XTRACE(DATA, DEB, "xoffset %d, xlocal %d, yoffset %d, ylocal %d, pixel %hu",
           xoff, xlocal, yoff, ylocal, pixel);
  }

  return pixel;
}

size_t BifrostGeometry::numSerializers() const {
  return TripletsPerRing *
         (MaxRing + 1); // MaxRing is likely 2 (but [0, 1, 2] are all valid)
}

size_t BifrostGeometry::calcSerializer(const DataParser::CaenReadout &Data) const {
  // FiberID = _physical_ Ring (logical_ring/2)
  // Group == triplet number
  return Data.FiberId / 2 * TripletsPerRing + Data.Group;
}

std::string BifrostGeometry::serializerName(size_t Index) const {
  auto ring_id = Index / TripletsPerRing;
  auto tube_id = Index - ring_id * TripletsPerRing;
  auto arc = tube_id / 3u;
  auto triplet = (tube_id % 3u) + ring_id * 3u;
  return fmt::format("arc={};triplet={}", arc, triplet);
}

} // namespace Caen
