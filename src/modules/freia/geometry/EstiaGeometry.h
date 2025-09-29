// Copyright (C) 2024 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Estia geometry class
///
/// Mapping from digital identifiers to x- and y- coordinates
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <freia/geometry/GeometryBase.h>
#include <freia/geometry/Config.h>
#include <limits>
#include <freia/Counters.h>
#include <common/readout/vmm3/VMM3Config.h>
#include <common/Statistics.h>

#include <cinttypes>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Freia {

class EstiaGeometry : public GeometryBase {
public:
  static constexpr uint32_t ESSGEOMTERY_NX = 1536;
  static constexpr uint32_t ESSGEOMTERY_NY = 128;
  static constexpr uint32_t ESSGEOMTERY_NZ = 1;
  static constexpr uint32_t ESSGEOMTERY_NP = 1;

  EstiaGeometry(Statistics &Stats, Config &Cfg)
      : GeometryBase(Stats, Cfg, VMM3Config::MaxRing, VMM3Config::MaxFEN,
                     ESSGEOMTERY_NX, ESSGEOMTERY_NY, ESSGEOMTERY_NZ,
                     ESSGEOMTERY_NP) {}

  bool validateReadoutData(const ESSReadout::VMM3Parser::VMM3Data &Data) override {
    uint8_t Ring = Data.FiberId / 2; // physical->logical mapping
    uint8_t HybridId = Data.VMM >> 1;
    
    return validateAll(
      [&]() {
        return validateRing(Ring);
      },
      [&]() {
        return validateFEN(Data.FENId);
      },
      [&]() {
        return validateHybrid(Ring, Data.FENId, HybridId);
      },
      [&]() {
        return validateChannel(Data.VMM, Data.Channel);
      }
    );
  }

protected:
  /// \brief Estia uses wires for X plane (different from default)
  bool inline usesWiresForX() const override { return true; }
  
  /// \brief Estia uses strips for Y plane (different from default) 
  bool inline usesWiresForY() const override { return false; }
  
  /// \brief Estia wire calculation: Offset + (Channel - MinWireChannel) or InvalidCoord if overflow
  uint16_t calcFromWire(uint16_t Offset, uint8_t Channel) const override {
    // Use uint32_t to detect overflow
    uint32_t result = static_cast<uint32_t>(Offset) + Channel - MinWireChannel;
    if (result > 65535) {
      XTRACE(DATA, WAR, "Coordinate overflow: %u + %u - %u = %u", 
             Offset, Channel, MinWireChannel, result);
      GeometryCounters.CoordOverflow++;
      return InvalidCoord;
    }
    return static_cast<uint16_t>(result);
  }


};

} // namespace Freia
