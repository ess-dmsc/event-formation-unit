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

namespace Dream {

class Cuboid {
public:

  ESSGeometry Geometry{112, 112 * 32, 1, 1};

  struct CuboidOffset {
    int X;
    int Y;
  };

  // clang-format off
  /// \brief Showing offsets roughly as the detector Cuboids are arranged
  std::vector<CuboidOffset> Offsets {
                        {32,  0}, {48,  0}, {64,  0},
              {16, 16}, {32, 16}, {48, 16}, {64, 16}, {80, 16},
    { 0, 32}, {16, 32}, {32, 32}, {48, 32}, {64, 32}, {80, 32}, {96, 32},
    { 0, 48}, {16, 48}, {32, 48},           {64, 48}, {80, 48}, {96, 48},
    { 0, 64}, {16, 64}, {32, 64},           {64, 64}, {80, 64}, {96, 64},
              {16, 80}, {32, 80},           {64, 80}, {80, 80},
                        {32, 96},           {64, 96}
  };
  // clang-format on


  /// \brief get global x-coordinate for Cuboid with a given index
  int getX(int Index, int Cassette, int Counter, int Wire, int Rotate) {
    /// \todo add XTRACE and counter
    if (Index >= (int)Offsets.size()) {
      return -1;
    }

    int LocalX = 2 * Cassette + Counter; // unrotated x,y values
    int LocalY = 15 - Wire;

    switch (Rotate) {
    case 1: // 90 deg. clockwise
      LocalX = 15 - LocalY;
      break;
    case 2: // 180 deg. cockwise
      LocalX = 15 - LocalX;
      break;
    case 3: // 270 deg. clockwise
      LocalX = LocalY;
      break;
    }

    return Offsets[Index].X + LocalX;
  }

  /// \brief get global y-coordinate for Cuboid with a given index
  int getY(int Index, int Cassette, int Counter, int Wire, int Strip,
                 int Rotate) {
    /// \todo add XTRACE and counter
    if (Index >= (int)Offsets.size()) {
     return -1;
    }

    int LocalX = 2 * Cassette + Counter; // unrotated x,y values
    int LocalY = 15 - Wire;

    switch (Rotate) {
    case 1: // 90 deg. clockwise
      LocalY = LocalX;
      break;
    case 2: // 180 deg. cockwise
      LocalY = 15 - LocalY;
      break;
    case 3: // 270 deg. clockwise
      LocalY = 15 - LocalX;
      break;
    }

    constexpr int YDim{7 * 16};
    return YDim * Strip + Offsets[Index].Y + LocalY;
  }


  //
  uint32_t getPixelId(Config::ModuleParms & Parms,
                    DataParser::DreamReadout & Data) {
  /// \todo fix and check all values
  uint8_t Index = Parms.P1.Index;
  uint8_t Cassette = 0;
  uint8_t Counter = 0;
  uint8_t Wire = Data.Cathode;
  uint8_t Strip = Data.Anode;
  uint8_t Rotate = Parms.P2.Rotate;

  int x = getX(Index, Cassette, Counter, Wire, Rotate);
  int y = getY(Index, Cassette, Counter, Wire, Strip, Rotate);
  return Geometry.pixel2D(x, y);
  }


};
} // namespace
