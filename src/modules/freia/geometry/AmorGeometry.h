// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief AMOR geometry class
///
/// Mapping from digital identifiers to x- and y- coordinates
//===----------------------------------------------------------------------===//

#pragma once

#include <common/Statistics.h>
#include <common/debug/Trace.h>
#include <common/readout/vmm3/VMM3Config.h>
#include <cstdint>
#include <freia/Counters.h>
#include <freia/geometry/Config.h>
#include <freia/geometry/GeometryBase.h>
#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Freia {

class AmorGeometry : public GeometryBase {
public:
  static constexpr uint32_t ESSGEOMTERY_NX = 64;
  static constexpr uint32_t ESSGEOMTERY_NY = 448;
  static constexpr uint32_t ESSGEOMTERY_NZ = 1;
  static constexpr uint32_t ESSGEOMTERY_NP = 1;

  AmorGeometry(Statistics &Stats, Config &Cfg)
      : GeometryBase(Stats, Cfg, VMM3Config::MaxRing, VMM3Config::MaxFEN,
                     ESSGEOMTERY_NX, ESSGEOMTERY_NY, ESSGEOMTERY_NZ,
                     ESSGEOMTERY_NP) {}

  /// \brief AMOR uses X on even VMM (opposite of base policy)
  bool isXCoord(uint8_t VMM) override { return !GeometryBase::isXCoord(VMM); }

  /// \brief Validate VMM3 readout data for AMOR geometry
  /// \param Data VMM3 readout data to validate
  /// \return true if readout is valid, false otherwise
  bool
  validateReadoutData(const ESSReadout::VMM3Parser::VMM3Data &Data) override {
    uint8_t Ring = Data.FiberId / 2; // physical->logical mapping
    uint8_t HybridId = Data.VMM >> 1;

    return validateAll(
        [&]() { return validateRing(Ring); },
        [&]() { return validateFEN(Data.FENId); },
        [&]() { return validateHybrid(Ring, Data.FENId, HybridId); },
        [&]() { return validateChannel(Data.VMM, Data.Channel); });
  }
};

} // namespace Freia
