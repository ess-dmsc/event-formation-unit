// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CSPEC geometry class
/// Based on CSPEC ICD documen
/// thttps://project.esss.dk/owncloud/index.php/f/14482406

/// Mapping from digital identifiers to x-, z- and y- coordinates
//===----------------------------------------------------------------------===//

#include <cspec/geometry/CSPECGeometry.h>

#include <cmath>
#include <utility>

uint16_t Cspec::CSPECGeometry::xAndzCoord(uint8_t FENID, uint8_t HybridID,
                                          uint8_t VMMID, uint8_t Channel,
                                          uint16_t XOffset, bool Rotated) {
  if (!validWireMapping(HybridID, VMMID, Channel)) {
    XTRACE(DATA, WAR,
           "Invalid Hybrid: %u, VMM: %u, and Channel: %u, combination for wire "
           "mapping",
           HybridID, VMMID, Channel);
    // return std::pair<uint8_t, uint8_t>(Cspec::CSPECGeometry::InvalidCoord,
    // Cspec::CSPECGeometry::InvalidCoord);
    return InvalidCoord;
  }

  // Wire equation defined in CSPEC ICD Document
  uint16_t xAndzCoordNumber = (HybridID * 2 + VMMID) * 64 + Channel - 32;

  uint8_t Depth = 16;
  uint8_t ColumnWidth = 6;
  uint8_t ColumnOffset = Depth * ColumnWidth;

  // odd FENs are second column in a vessel, 6 * 16 wires over
  if (FENID % 2) {
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

uint16_t Cspec::CSPECGeometry::yCoord(uint8_t HybridID, uint8_t VMMID,
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
bool Cspec::CSPECGeometry::validGridMapping(uint8_t HybridID, uint8_t VMMID,
                                            uint8_t Channel, bool Short) {
  if (Short) {
    return (HybridID == 1 and VMMID == 0 and Channel <= 50);
  }
  if (HybridID == 1) {
    if (VMMID == 0) {
      return (Channel >= 58 and Channel <= 63);
    } else if (VMMID != 1) { // hybrid 1 has 2 VMMs only, VMM0 and VMM1
      return false;
    }
  } else if (HybridID == 2) {
    if (VMMID == 1) {
      return Channel <= 5;
    } else if (VMMID != 0) { // hybrid 2 has 2 VMMs only, VMM0 and VMM1
      return false;
    }
  } // hybrid id for grids must be 1 or 2
  return false;
}

// The valid combinations of these parameters are defined in CSPEC ICD document
bool Cspec::CSPECGeometry::validWireMapping(uint8_t HybridID, uint8_t VMMID,
                                            uint8_t Channel) {
  if (HybridID != 0) {
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
