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

#include <cmath>
#include <utility>

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
  return CoordNumber;
}

