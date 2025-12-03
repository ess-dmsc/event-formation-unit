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
#include <dream/geometry/GeometryModule.h>
#include <dream/readout/DataParser.h>
#include <logical_geometry/ESSGeometry.h>
#include <stdint.h>
#include <string>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_WAR

namespace Dream {

class Cuboid : public ESSGeometry, public DreamSubModule {
private:
  constexpr static uint32_t MAX_X = 112;
  constexpr static uint32_t MAX_Y = 112 * 32;
  constexpr static uint32_t MAX_Z = 1;
  constexpr static uint32_t MAX_PIXEL = 1;
  constexpr static uint8_t WIRES_PER_COUNTER = 16;
  constexpr static uint8_t STRIPS_PER_CASSETTE = 32;

  struct CuboidCounters : public StatCounterBase {
    int64_t IndexErrors{0};
    int64_t TypeErrors{0};

    CuboidCounters(Statistics &Stats, const std::string &Name)
        : StatCounterBase(Stats,
                          {
                              {METRIC_COUNTER_INDEX_ERRORS, IndexErrors},
                              {METRIC_COUNTER_TYPE_ERRORS, TypeErrors},
                          },
                          Name) {}
  };

  mutable CuboidCounters CuboidCounters;

public:
  struct CuboidOffset {
    int X;
    int Y;
  };

  // clang-format off
  static inline const std::string METRIC_COUNTER_INDEX_ERRORS = "geometry.index_errors";
  static inline const std::string METRIC_COUNTER_TYPE_ERRORS  = "geometry.type_errors";

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

  Cuboid(Statistics &Stats, const std::string &Name)
      : ESSGeometry(MAX_X, MAX_Y, MAX_Z, MAX_PIXEL), CuboidCounters(Stats, Name) {}

  /// \brief Rotate local x,y coordinates according to module rotation
  /// \param Rotate Rotation index (0-3)
  /// \param LocalX Local x-coordinate to rotate (input and output)
  /// \param LocalY Local y-coordinate to rotate (input and output)
  void rotateXY(int Rotate, int &LocalX, int &LocalY) const;

  /// \brief get pixel id from CDT readout data and module parameters
  /// \param Parms Const reference to module parameters
  /// \param Data Const reference to CDT readout data
  /// \return Calculated pixel ID, or 0 if calculation failed
  uint32_t calcPixelId(const Config::ModuleParms &Parms,
                       const DataParser::CDTReadout &Data) const override;

  const struct CuboidCounters &getCuboidCounters() const {
    return CuboidCounters;
  }
};
} // namespace Dream
