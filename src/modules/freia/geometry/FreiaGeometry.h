// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Freia geometry class
///
/// Mapping from digital identifiers to x- and y- coordinates
//===----------------------------------------------------------------------===//

#pragma once

#include <common/Statistics.h>
#include <common/debug/Trace.h>
#include <common/geometry/vmm3/VMM3Geometry.h>
#include <common/readout/vmm3/VMM3Config.h>
#include <freia/Counters.h>
#include <freia/geometry/Config.h>

#include <cinttypes>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Freia {

class FreiaGeometry final : public vmm3::VMM3Geometry {
public:
  static constexpr uint32_t ESSGEOMTERY_NX = 64;
  static constexpr uint32_t ESSGEOMTERY_NY = 1024;
  static constexpr uint32_t ESSGEOMTERY_NZ = 1;
  static constexpr uint32_t ESSGEOMTERY_NP = 1;

  FreiaGeometry(Statistics &Stats, Config &Cfg)
      : vmm3::VMM3Geometry(Stats, Cfg, vmm3::VMM3Config::MaxRing,
                           vmm3::VMM3Config::MaxFEN, ESSGEOMTERY_NX,
                           ESSGEOMTERY_NY, ESSGEOMTERY_NZ, ESSGEOMTERY_NP) {}

  /// \brief Validate VMM3 readout data for Freia geometry
  /// \param Data VMM3 readout data to validate
  /// \return true if valid, false otherwise
  bool validateReadoutData(const vmm3::VMM3Parser::VMM3Data &Data) override {
    uint8_t Ring = Data.FiberId / 2; // physical->logical mapping
    uint8_t HybridId = calcHybridId(Data.VMM);

    return validateAll(
        [&]() { return validateRing(Ring); },
        [&]() { return validateFEN(Data.FENId); },
        [&]() { return validateHybrid(Ring, Data.FENId, HybridId); },
        [&]() { return validateChannel(Data.VMM, Data.Channel); });
  }
};

} // namespace Freia
