// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CSPEC geometry class
/// Based on CSPEC ICD document
/// thttps://project.esss.dk/owncloud/index.php/f/14482406

/// Mapping from digital identifiers to x-, z- and y- coordinates
//===----------------------------------------------------------------------===//

#include <cspec/geometry/LETGeometry.h>

#include <cmath>
#include <utility>

// returns integer describing the X and Z position in the flattened 2D space
uint16_t Cspec::LETGeometry::xAndzCoord(uint8_t RingID, uint8_t FENID,
                                        uint8_t HybridID, uint8_t VMMID,
                                        uint8_t Channel, uint16_t XOffset,
                                        bool Rotated) {

  if (Channel % 2 == 0) {
    Channel = Channel + 1;
  } else {
    Channel = Channel - 1;
  }

  if (!validWireMapping(HybridID, VMMID, Channel)) {
    XTRACE(DATA, WAR,
           "Invalid Hybrid: %u, VMM: %u, and Channel: %u, combination for wire "
           "mapping",
           HybridID, VMMID, Channel);
    return InvalidCoord;
  }
  XTRACE(DATA, DEB, "FEN ID is %d", FENID);

  // Wire equation defined in CSPEC ICD Document
  uint16_t xAndzCoordNumber = VMMID * 64 + Channel - 32;

  uint8_t Depth = 16;
  uint8_t ColumnWidth = 6;
  uint8_t ColumnOffset = Depth * ColumnWidth;

  // even Rings are second column in a vessel, 6 * 16 wires over
  if (!(RingID % 2)) {
    xAndzCoordNumber = xAndzCoordNumber + ColumnOffset;
  }

  // when rotated local X coordinate is flipped around centre, ie 11-XCoord
  // Z coord remains the same
  // Then turning value back into wire value/combination of X and Z
  if (Rotated) {
    uint8_t LocalXCoord = xAndzCoordNumber >> 4;
    uint8_t LocalZCoord = xAndzCoordNumber % Depth;
    LocalXCoord = 11 - LocalXCoord;
    xAndzCoordNumber = Depth * LocalXCoord + LocalZCoord;
  }

  xAndzCoordNumber += Depth * XOffset;
  return xAndzCoordNumber;
}

uint16_t Cspec::LETGeometry::yCoord(uint8_t HybridID, uint8_t VMMID,
                                    uint8_t Channel, uint16_t YOffset,
                                    bool Rotated, bool Short) {
  uint16_t YCoord;
  if (!validGridMapping(HybridID, VMMID, Channel, Short)) {
    XTRACE(DATA, ERR,
           "Invalid combination of HybridID: %u, VMMID: %u, Channel: %u, "
           "Short: %d",
           HybridID, VMMID, Channel, Short);
    return InvalidCoord;
  }

  // Channel mappings for Y coordinates/grids are detailed in ICD document
  // equation used is equation for YCoord in section 3.4.4
  if (!Short) { // channel mapping for full length vessel
    YCoord = (HybridID * 2 + VMMID - 2) * 64 + Channel - 58;
  } else { // channel mapping for short vessel
    YCoord = Channel;
  }

  // when rotated grid order is reversed, and YOffset should be used to place
  // vessel correctly
  if (Rotated) {
    YCoord = -YCoord;
  }

  YCoord += YOffset;
  return YCoord;
}

// The valid combinations of these parameters are defined in CSPEC ICD document
bool Cspec::LETGeometry::validGridMapping(uint8_t HybridID, uint8_t VMMID,
                                          uint8_t Channel, bool Short) {
  if (Short) {
    return (HybridID == 0 and VMMID == 0 and Channel <= 50);
  }
  if (HybridID == 1) {
    if (VMMID == 0) {
      return (Channel >= 58 and Channel <= 63);
    } else if (VMMID == 1) { // hybrid 1 has 2 VMMs only, VMM0 and VMM1
      return true;
    }
  } else if (HybridID == 2) {
    if (VMMID == 1) {
      return Channel <= 5;
    } else if (VMMID == 0) { // hybrid 2 has 2 VMMs only, VMM0 and VMM1
      return true;
    }
  } // hybrid id for grids must be 1 or 2
  return false;
}

// The valid combinations of these parameters are defined in CSPEC ICD document
bool Cspec::LETGeometry::validWireMapping(uint8_t HybridID, uint8_t VMMID,
                                          uint8_t Channel) {
  if (HybridID != 1) {
    return false;
  }
  if (VMMID == 0) { // only channels 32-63 used on VMM0
    return Channel >= 32 and Channel <= 63;
  } else if (VMMID == 1) { // channels only go up to 63
    return Channel <= 63;
  } else {
    return false;
  }
}
