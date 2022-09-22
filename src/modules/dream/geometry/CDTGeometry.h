// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Attempt to handle the complex DREAM geometry which is assembled
/// from five different CDT module types with different characeteristics
///
/// Heavily uses the formulae and parameters from the DREAM ICD which can be
/// located through
/// https://confluence.esss.lu.se/display/ECDC/Instrument+Status+Overview
//===----------------------------------------------------------------------===//

#pragma once

#include <dream/geometry/Config.h>

namespace Dream {

class CDTGeometry {
public:

  /// \brief return the global pixel id offset for each of the DREAM detector
  /// components. This offset must be added to the local pixel id calculated
  /// for that module (see ICD for full description)
  int getPixelOffset(Config::ModuleType Type);


  /// \brief return local pixel id from the digital identifiers
  int getLocalPixel(uint8_t Ring, uint8_t FEN, Config::ModuleType Type,
      uint8_t Cathode, uint8_t Anode);
};
}

/*
#include <common/debug/Assert.h>
#include <common/debug/Trace.h>
#include <logical_geometry/ESSGeometry.h>
#include <stdint.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_WAR

namespace Dream {

struct Offset {
  int X;
  int Y;
};

// clang-format off
// Showing offsets roughly as the detector Cuboids are arranged
Offset Offsets[] {    {32,  0}, {48,  0}, {64,  0},
            {16, 16}, {32, 16}, {48, 16}, {64, 16}, {80, 16},
  { 0, 32}, {16, 32}, {32, 32}, {48, 32}, {64, 32}, {80, 32}, {96, 32},
  { 0, 48}, {16, 48}, {32, 48},           {64, 48}, {80, 48}, {96, 48},
  { 0, 64}, {16, 64}, {32, 64},           {64, 64}, {80, 64}, {96, 64},
            {16, 80}, {32, 80},           {64, 80}, {80, 80},
                      {32, 96},           {64, 96}
};
// clang-format on

class CDTGeometry {
public:
  enum ModuleType { Undefined, Mantle, Cuboid, SUMO3, SUMO4, SUMO5, SUMO6 };

  CDTGeometry(ModuleType Type, int Cassettes, int Wires, int Strips, bool Sumo,
              int Xoffset)
      : Type(Type), Cassettes(Cassettes), Wires(Wires), Strips(Strips),
        Sumo(Sumo), Xoffset(Xoffset) {}

  /// Works for SUMO 3 - 6
  int getXSUMO(int Sector, int Cassette, int Counter) {
    return 56 * Sector + Xoffset + 2 * Cassette + Counter;
  }

  /// Works for SUMO 3 - 6
  int getYSUMO(int Wire, int Strip) { return 16 * Strip + 15 - Wire; }

  /// \todo range checking for array indexing
  int getXCuboid(int Index, int Cassette, int Counter, int Wire, int Rotate) {
    int LocalX = 2 * Cassette + Counter;
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

  int getYCuboid(int Index, int Cassette, int Counter, int Wire, int Strip,
                 int Rotate) {
    int LocalX = 2 * Cassette + Counter;
    int LocalY = 15 - Wire;
    // printf("Local X/Y %d, %d\n", LocalX, LocalY);
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
    // printf("Final Y %d\n", LocalY);
    constexpr int YDim{7 * 16};
    return YDim * Strip + Offsets[Index].Y + LocalY;
  }

public:
  ModuleType Type{Undefined};
  int Cassettes{0};
  int Wires{0};
  int Strips{0};
  bool Sumo{true}; // if true, wires and strips encodes z and y positions
  int Rotate{0};   // 0 == 0 degrees, 1 = 90 degrees, 2 == 180 degrees, 3 == 270
  int Xoffset{0};
}; 
*/
