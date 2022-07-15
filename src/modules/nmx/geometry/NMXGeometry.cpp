// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief NMX geometry class
/// Based on NMX ICD document

/// Mapping from digital identifiers to x-, z- and y- coordinates
//===----------------------------------------------------------------------===//

#include <nmx/geometry/NMXGeometry.h>
#include <common/debug/Trace.h>

#include <cmath>
#include <utility>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

// returns integer describing the X and Z position in the flattened 2D space
uint16_t Nmx::NMXGeometry::coord(uint8_t Channel, uint8_t AsicId, uint16_t Offset, bool ReversedChannels) {
  uint16_t CoordNumber;

  // Wire equation defined in NMX ICD Document
  if (ReversedChannels){
    CoordNumber = Offset + 64 * AsicId + 63 - Channel;
  }
  else{
    CoordNumber = Offset + 64 * AsicId + Channel;
  }
  XTRACE(DATA, DEB, "Calculating coordinate value from Channel: %u, AsicId: %u, Offset: %u, ReversedChannels: %B, got Coordinate %u", Channel, AsidId, Offset, ReversedChannels, CoordNumber);

  return CoordNumber;
}

