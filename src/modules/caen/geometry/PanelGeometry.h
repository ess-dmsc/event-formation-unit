/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
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
  static const uint8_t NStraws{7};       /// straws per tube
  const uint32_t StrawError{0xFFFFFFFF}; // return value upon error

  /// MaxGroup is equivalient to number of FENs
  uint32_t getMaxGroup() { return MaxGroup; }

  /// It is expected to have multiple instansiations of PanelGeometry, one
  /// one and sometimes more than one for each panel. It is the assumption that
  /// RingId maps directly to a PanelGeometry object. Thus this implementation
  /// does not need to use RingId in its calculations.
  PanelGeometry(uint8_t TubesZ, uint8_t TubesN, uint32_t StrawOffset)
      : TZ(TubesZ), TN(TubesN), StrawOffset(StrawOffset) {
    MaxStraw = TubesZ * TubesN * NStraws;
    MaxGroup = TubesN / 2;
  };

  /// \brief
  uint32_t getGlobalStrawId(uint8_t TubeGroup, uint8_t LocalTube,
                            uint16_t Straw) {
    if (TubeGroup >= MaxGroup) {
      XTRACE(EVENT, WAR, "Invalid TubeGroup %d (max %d)", TubeGroup, MaxGroup);
      return StrawError;
    }
    if (LocalTube >= 8) {
      XTRACE(EVENT, WAR, "Invalid LocalTube %d (max %d)", LocalTube, 8);
      return StrawError;
    }

    if (Straw >= NStraws) {
      XTRACE(EVENT, WAR, "Invalid Straw %d (max %d)", Straw, NStraws);
      return StrawError;
    }
    /// (0) (1) (2) (3)
    /// (4) (5) (6) (7)
    auto TubeLayer = LocalTube % TZ; /// 0 - 3
    auto TubeIndex = LocalTube / TZ; /// 0, 1

    auto LayerOffset = NStraws * TN * TubeLayer;
    auto GroupOffset = TubeGroup * 2 * NStraws;
    auto TubeOffset = TubeIndex * NStraws;

    // Add the contributions to the total straw - this is the y coordinate
    // for the pixelid calculation, x comes from the position along the straw
    auto AbsoluteStraw =
        StrawOffset + LayerOffset + GroupOffset + TubeOffset + Straw;
    assert(AbsoluteStraw < MaxStraw + StrawOffset);
    return AbsoluteStraw;
  }

private:
  ///< Initialised in constructor
  uint8_t TZ{0}; ///< Tubes in the z-direction
  uint8_t TN{0}; ///< Tubes in the y/x direction depending on orientation
  uint32_t StrawOffset{0}; ///< global straw number for first straw in bank
  uint32_t MaxStraw{0};
  uint32_t MaxGroup{0};
};
} // namespace Caen
