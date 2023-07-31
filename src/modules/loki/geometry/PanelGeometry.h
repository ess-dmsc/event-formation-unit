// Copyright (C) 2019 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Caen Panel Geometry
///
//===----------------------------------------------------------------------===//

#pragma once

#include <assert.h>
#include <common/debug/Trace.h>
#include <cstdint>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

class PanelGeometry {
public:
  static const uint8_t NUnits{7};       /// straws per tube
  const uint32_t UnitError{0xFFFFFFFF}; // return value upon error

  /// MaxGroup is equivalient to number of FENs
  uint32_t getMaxGroup() { return MaxGroup; }

  /// It is expected to have multiple instansiations of PanelGeometry, one
  /// and sometimes more than one for each panel. It is the assumption that
  /// Ring maps directly to a PanelGeometry object. Thus this implementation
  /// does not need to use Ring in its calculations.
  PanelGeometry(uint8_t GroupsZ, uint8_t GroupsN, uint32_t UnitOffset)
      : TZ(GroupsZ), TN(GroupsN), UnitOffset(UnitOffset) {
    MaxUnit = GroupsZ * GroupsN * NUnits;
    MaxGroup = GroupsN / 2;
  };

  /// \brief
  uint32_t getGlobalUnitId(uint8_t GroupBank, uint8_t Group, uint16_t Unit) {
    if (GroupBank >= MaxGroup) {
      XTRACE(EVENT, WAR, "Invalid GroupBank %d (max %d)", GroupBank, MaxGroup);
      return UnitError;
    }
    if (Group >= 8) {
      XTRACE(EVENT, WAR, "Invalid Group %d (max %d)", Group, 8);
      return UnitError;
    }

    if (Unit >= NUnits) {
      XTRACE(EVENT, WAR, "Invalid Unit %d (max %d)", Unit, NUnits);
      return UnitError;
    }
    XTRACE(EVENT, DEB, "Group: %u, TZ: %u, NUnits: %u, TN: %u", Group,
           TZ, NUnits, TN);
    /// (0) (1) (2) (3)
    /// (4) (5) (6) (7)
    auto GroupLayer = Group % TZ; /// 0 - 3
    auto GroupIndex = Group / TZ; /// 0, 1

    auto LayerOffset = NUnits * TN * GroupLayer;
    auto GroupOffset = GroupBank * 2 * NUnits;
    auto TubeOffset = GroupIndex * NUnits;

    // Add the contributions to the total straw - this is the y coordinate
    // for the pixelid calculation, x comes from the position along the straw
    auto AbsoluteUnit =
        UnitOffset + LayerOffset + GroupOffset + TubeOffset + Unit;
    assert(AbsoluteUnit < MaxUnit + UnitOffset);
    return AbsoluteUnit;
  }

private:
  ///< Initialised in constructor
  uint8_t TZ{0}; ///< Tubes in the z-direction
  uint8_t TN{0}; ///< Tubes in the y/x direction depending on orientation
  uint32_t UnitOffset{0}; ///< global straw number for first straw in bank
  uint32_t MaxUnit{0};
  uint32_t MaxGroup{0};
};
} // namespace Caen
