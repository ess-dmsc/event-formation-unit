// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Freia geometry base class implementation
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <freia/geometry/GeometryBase.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_INF

namespace Freia {

GeometryBase::CoordResult GeometryBase::calculateCoordinate(uint16_t XOffset,
                                                            uint16_t YOffset,
                                                            uint8_t VMM,
                                                            uint8_t Channel) {
  // Determine which coordinate plane this VMM contributes to
  bool isX = isXCoord(VMM);

  // Select the appropriate offset and coordinate type based on plane
  uint16_t offset = isX ? XOffset : YOffset;
  bool usesWires = isX ? usesWiresForX() : usesWiresForY();

  // Calculate coordinate using the appropriate method
  // Channel validation happens in validateReadoutData during normal processing
  uint16_t coordinate = usesWires ? calcFromWire(offset, Channel)
                                  : calcFromStrip(offset, Channel);

  // Check for calculation overflow and increment counter if needed
  if (coordinate == InvalidCoord) {
    incrementErrorCounter(isX);
  }

  XTRACE(DATA, INF,
         "Calculated %s coordinate: %u (Offset %u, Channel %u, "
         "%s-based)",
         isX ? "X" : "Y", coordinate, offset, Channel,
         usesWires ? "Wire" : "Strip");

  return {coordinate, isX};
}

bool GeometryBase::validateChannel(uint8_t VMM, uint8_t Channel) {
  bool isX = isXCoord(VMM);
  bool usesWires = isX ? usesWiresForX() : usesWiresForY();

  if (usesWires) {
    if ((Channel < MinWireChannel) || (Channel > MaxWireChannel)) {
      XTRACE(DATA, WAR, "Invalid Channel %d (%d <= ch <= %d)", Channel,
             MinWireChannel, MaxWireChannel);
      GeometryCounters.WireChannelRangeErrors++;
      // Also increment the general coordinate error counter for backward
      // compatibility
      incrementErrorCounter(isX);
      return false;
    }
  } else {
    if (Channel >= NumStrips) {
      XTRACE(DATA, WAR, "Invalid Channel %d (Max %d)", Channel, NumStrips - 1);
      GeometryCounters.StripChannelRangeErrors++;
      // Also increment the general coordinate error counter for backward
      // compatibility
      incrementErrorCounter(isX);
      return false;
    }
  }
  return true;
}

bool GeometryBase::validateHybrid(uint8_t Ring, uint8_t FENId,
                                  uint8_t HybridId) {
  auto &Hybrid = Conf.getHybrid(Ring, FENId, HybridId);
  if (!Hybrid.Initialised) {
    XTRACE(DATA, WAR,
           "Hybrid for Ring %d, FEN %d, VMM %d not defined in config file",
           Ring, FENId, (HybridId << 1));
    GeometryCounters.HybridMappingErrors++;
    return false;
  }
  return true;
}

// Static member definitions
uint16_t const GeometryBase::InvalidCoord = 0xFFFF;
uint16_t const GeometryBase::NumStrips = 64;
uint16_t const GeometryBase::NumWires = 32;
uint16_t const GeometryBase::MinWireChannel = 16;
uint16_t const GeometryBase::MaxWireChannel = 47;

} // namespace Freia
