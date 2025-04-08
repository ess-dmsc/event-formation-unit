// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Bitmaps used to mask active neutron detector areas
///
/// - The three space invader monsters in both of their two possible
///   physical configs. Individual invaders were cropped from the sprite taken
///   from here
///
///     https://www.spriters-resource.com/fullview/115520
///
/// - Lower case version of the greek letter zeta taken from the Computer
///   Modern font family
///
/// - Letters from A - Z and numbers 0 - 9
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START
#pragma once

#include <common/testutils/bitmaps/BitMap.h>

#include <cstdint>
#include <utility>
#include <vector>

// clang-format off
namespace BitMaps {
  class Bitmap : public bmp::Bitmap {
   public:
    Bitmap()
      : bmp::Bitmap(1, 1) {
    }

    Bitmap(const std::vector<std::pair<uint8_t, uint8_t>> &XY)
      : bmp::Bitmap(XY[0].first, XY[0].second) {
      clear(bmp::White);
      for (size_t i=1; i<XY.size(); ++i) {
        const auto &[x, y] = XY[i];
        set(x, y, bmp::Black);
      }
    }
  };

  // Space invaders
  extern const std::vector<std::pair<uint8_t, uint8_t>> si1_0;
  extern const std::vector<std::pair<uint8_t, uint8_t>> si1_1;
  extern const std::vector<std::pair<uint8_t, uint8_t>> si2_0;
  extern const std::vector<std::pair<uint8_t, uint8_t>> si2_1;
  extern const std::vector<std::pair<uint8_t, uint8_t>> si3_0;
  extern const std::vector<std::pair<uint8_t, uint8_t>> si3_1;

  // Lower case greek zeta - ζ
  extern const std::vector<std::pair<uint8_t, uint8_t>> zeta;

  // Letters - A to Z
  extern const std::vector<std::pair<uint8_t, uint8_t>> A;
  extern const std::vector<std::pair<uint8_t, uint8_t>> B;
  extern const std::vector<std::pair<uint8_t, uint8_t>> C;
  extern const std::vector<std::pair<uint8_t, uint8_t>> D;
  extern const std::vector<std::pair<uint8_t, uint8_t>> E;
  extern const std::vector<std::pair<uint8_t, uint8_t>> F;
  extern const std::vector<std::pair<uint8_t, uint8_t>> G;
  extern const std::vector<std::pair<uint8_t, uint8_t>> H;
  extern const std::vector<std::pair<uint8_t, uint8_t>> I;
  extern const std::vector<std::pair<uint8_t, uint8_t>> J;
  extern const std::vector<std::pair<uint8_t, uint8_t>> K;
  extern const std::vector<std::pair<uint8_t, uint8_t>> L;
  extern const std::vector<std::pair<uint8_t, uint8_t>> M;
  extern const std::vector<std::pair<uint8_t, uint8_t>> N;
  extern const std::vector<std::pair<uint8_t, uint8_t>> O;
  extern const std::vector<std::pair<uint8_t, uint8_t>> P;
  extern const std::vector<std::pair<uint8_t, uint8_t>> Q;
  extern const std::vector<std::pair<uint8_t, uint8_t>> R;
  extern const std::vector<std::pair<uint8_t, uint8_t>> S;
  extern const std::vector<std::pair<uint8_t, uint8_t>> T;
  extern const std::vector<std::pair<uint8_t, uint8_t>> U;
  extern const std::vector<std::pair<uint8_t, uint8_t>> V;
  extern const std::vector<std::pair<uint8_t, uint8_t>> W;
  extern const std::vector<std::pair<uint8_t, uint8_t>> X;
  extern const std::vector<std::pair<uint8_t, uint8_t>> Y;
  extern const std::vector<std::pair<uint8_t, uint8_t>> Z;
  extern const std::vector<std::pair<uint8_t, uint8_t>> Space;

  // Numbers - 0 to 9
  extern const std::vector<std::pair<uint8_t, uint8_t>> N0;
  extern const std::vector<std::pair<uint8_t, uint8_t>> N1;
  extern const std::vector<std::pair<uint8_t, uint8_t>> N2;
  extern const std::vector<std::pair<uint8_t, uint8_t>> N3;
  extern const std::vector<std::pair<uint8_t, uint8_t>> N4;
  extern const std::vector<std::pair<uint8_t, uint8_t>> N5;
  extern const std::vector<std::pair<uint8_t, uint8_t>> N6;
  extern const std::vector<std::pair<uint8_t, uint8_t>> N7;
  extern const std::vector<std::pair<uint8_t, uint8_t>> N8;
  extern const std::vector<std::pair<uint8_t, uint8_t>> N9;
} // namespace BitMaps

// clang-format on

// GCOVR_EXCL_STOP
