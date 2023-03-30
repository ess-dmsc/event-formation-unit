// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CDT Cuboid module abstractions
///
/// Consult ICD for logical geometry dimensions, rotations etc.
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <dream/geometry/Config.h>
#include <dream/readout/DataParser.h>
#include <logical_geometry/ESSGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_WAR

namespace Dream {

class Cuboid {
public:
  ESSGeometry Geometry{112, 112 * 32, 1, 1};

  const uint8_t WiresPerCounter{16};
  const uint8_t StripsPerCass{32};

  struct CuboidOffset {
    int X;
    int Y;
  };

  // clang-format off
  /// \brief Showing offsets roughly as the detector Cuboids are arranged
  ///
  std::vector<CuboidOffset> OffsetsHR {
                        {32,  0}, {48,  0}, {64,  0},
              {16, 16}, {32, 16}, {48, 16}, {64, 16}, {80, 16},
    { 0, 32}, {16, 32}, {32, 32}, {48, 32}, {64, 32}, {80, 32}, {96, 32},
    { 0, 48}, {16, 48}, {32, 48},           {64, 48}, {80, 48}, {96, 48},
    { 0, 64}, {16, 64}, {32, 64},           {64, 64}, {80, 64}, {96, 64},
              {16, 80}, {32, 80},           {64, 80}, {80, 80},
                        {32, 96},           {64, 96}
  };

  std::vector<int> RotateHR = {
        0, 0, 1,
     0, 0, 0, 1, 1,
  0, 0, 0, 0, 1, 1, 1,
  3, 3, 3,    1, 1, 1,
  3, 3, 3,    2, 2, 2,
     3, 3,    2, 2,
        3,    2
  };

  std::vector<CuboidOffset> OffsetsSANS {
                        {32,  0}, {48,  0}, {64,  0},
              {16, 16}, {32, 16}, {48, 16}, {64, 16}, {80, 16},
    { 0, 32}, {16, 32}, {32, 32}, {48, 32}, {64, 32}, {80, 32}, {96, 32},
    { 0, 48}, {16, 48}, {32, 48},           {64, 48}, {80, 48}, {96, 48},
    { 0, 64}, {16, 64}, {32, 64}, {48, 64}, {64, 64}, {80, 64}, {96, 64},
              {16, 80}, {32, 80}, {48, 80}, {64, 80}, {80, 80},
                        {32, 96}, {48, 96}, {64, 96}
  };


  std::vector<int> RotateSANS = {
        0, 0, 1,
     0, 0, 0, 1, 1,
  0, 0, 0, 0, 1, 1, 1,
  3, 3, 3,    1, 1, 1,
  3, 3, 3, 2, 2, 2, 2,
     3, 3, 2, 2, 2,
        3, 2, 2
  };

  // clang-format on

  /// \brief rotate (x,y)
  void rotateXY(int &LocalX, int &LocalY, int Rotate) {
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

  //
  uint32_t getPixelId(Config::ModuleParms &Parms,
                      DataParser::DreamReadout &Data) {
    uint8_t Index = Parms.P1.Index;
    Index += Data.Unused; // used as instance

    XTRACE(DATA, DEB, "index %u, anode %u, cathode %u", Index, Data.Anode,
           Data.Cathode);
    uint8_t Cassette = Data.Anode / 32 + 2 * (Data.Cathode / 32);
    uint8_t Counter = (Data.Anode / WiresPerCounter) % 2;
    uint8_t Wire = Data.Anode % WiresPerCounter;
    uint8_t Strip = Data.Cathode % StripsPerCass;

    XTRACE(DATA, DEB, "cass %u, ctr %u, wire %u, strip %u", Cassette, Counter,
           Wire, Strip);

    CuboidOffset Offset;
    int Rotation;
    if (Parms.Type == Config::ModuleType::SANS) {
      if (Index >= (int)OffsetsSANS.size()) {
        XTRACE(DATA, WAR, "Bad SANS index %u", Index);
        return -1;
      }
      Offset = OffsetsSANS[Index];
      Rotation = RotateSANS[Index];
    } else if (Parms.Type == Config::ModuleType::HR) {
      if (Index >= (int)OffsetsHR.size()) {
        XTRACE(DATA, WAR, "Bad HR index %u", Index);
        return -1;
      }
      Offset = OffsetsHR[Index];
      Rotation = RotateHR[Index];
    } else {
      XTRACE(DATA, WAR, "Inconsistent type (%d) for Cuboid", Parms.Type);
      return -1;
    }

    int LocalX = 2 * Cassette + Counter; // unrotated x,y values
    int LocalY = 15 - Wire;

    XTRACE(DATA, DEB, "local x %u, local y %u, rotate %u", LocalX, LocalY,
           Rotation);

    rotateXY(LocalX, LocalY, Rotation);

    constexpr int YDim{7 * 16};
    int x = Offset.X + LocalX;
    int y = YDim * Strip + Offset.Y + LocalY;
    XTRACE(DATA, DEB, "x %u, y %u", x, y);

    return Geometry.pixel2D(x, y);
  }
};
} // namespace Dream
