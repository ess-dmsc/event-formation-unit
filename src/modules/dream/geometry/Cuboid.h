// Copyright (C) 2022 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CDT Cuboid module abstractions
///
/// Consult ICD for logical geometry dimensions, rotations etc.
//===----------------------------------------------------------------------===//

#pragma once

#include <common/Statistics.h>
#include <common/debug/Trace.h>
#include <cstdint>
#include <dream/geometry/Config.h>
#include <dream/readout/DataParser.h>
#include <logical_geometry/ESSGeometry.h>
#include <stdint.h>
#include <string>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_WAR

namespace Dream {

class Cuboid : public ESSGeometry {
private:
  constexpr static uint8_t WIRES_PER_COUNTER = 16;
  constexpr static uint8_t STRIPS_PER_CASSETTE = 32;

  struct CuboidCounters : public StatCounterBase {
    int64_t IndexErrors{0};
    int64_t TypeErrors{0};

    CuboidCounters(Statistics &Stats)
        : StatCounterBase(Stats,
                          {
                              {METRIC_COUNTER_INDEX_ERRORS, IndexErrors},
                              {METRIC_COUNTER_TYPE_ERRORS, TypeErrors},
                          },
                          "cuboid") {}
  };

  mutable CuboidCounters CuboidCounters;

public:
  // clang-format off
  static inline const std::string METRIC_COUNTER_INDEX_ERRORS = "geometry.index_errors";
  static inline const std::string METRIC_COUNTER_TYPE_ERRORS  = "geometry.type_errors";
  // clang-format on

  Cuboid(Statistics &Stats)
      : ESSGeometry(112, 112 * 32, 1, 1), CuboidCounters(Stats) {}

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
        0, 1, 1,
     0, 0, 1, 1, 1,
  0, 0, 0, 1, 1, 1, 1,
  0, 0, 0,    2, 2, 2,
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
        0, 1, 1,
     0, 0, 1, 1, 1,
  0, 0, 0, 1, 1, 1, 1,
  0, 0, 0,    2, 2, 2,
  3, 3, 3, 3, 2, 2, 2,
     3, 3, 3, 2, 2,
        3, 3, 2
  };

  // clang-format on

  /// \brief rotate (x,y)
  void rotateXY(int &LocalX, int &LocalY, int Rotate) const;

  /// \brief get pixel id from CDT readout data
  uint32_t calcPixelId(const Config::ModuleParms &Parms,
                       const DataParser::CDTReadout &Data) const;

  const struct CuboidCounters &getCuboidCounters() const {
    return CuboidCounters;
  }
};
} // namespace Dream
