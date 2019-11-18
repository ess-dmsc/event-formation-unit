/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Loki Panel Geometry
///
/// Currently this will only support 2D geometry
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/Trace.h>
#include <logical_geometry/ESSGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Loki {
class PanelGeometry {
public:

  /// It is expected to have multiple instansiation of PanelGeometry, one
  /// one for each panel. It is the assumption that RingId maps directly to
  /// a PanelGeometry object. Thus this implementation does not need to use
  /// RingId in its calculations.
  PanelGeometry(bool VerticalTubes, uint8_t TubesZ, uint8_t TubesN, uint32_t PixelOffset) :
    Vertical(VerticalTubes), TZ(TubesZ), TN(TubesN), Offset(PixelOffset) {
    uint16_t NX{0};
    uint16_t NY{0};
    if (Vertical) {
      NX = TN * NStraws * TZ;
      NY = NPos;
    } else {
      NX = NPos;
      NY = TN * NStraws * TZ;
    }
    XTRACE(INIT, DEB, "ESSGeometry(%u, %u, %u, %u)", NX, NY, 1, 1);
    Geometry2D = ESSGeometry(NX, NY, 1, 1);
  };

  /// \brief Calculate PixelId for a readout
  /// Transformation from canonical (vertical) to rotated (horizontal) is
  /// (x,y) -> (NPos - 1 - y, x)
  uint32_t getPixel2D(uint8_t __attribute__((unused)) FENId, uint8_t FPGAId,
      uint8_t LocalTube, uint16_t StrawId, uint16_t Pos) {
    uint16_t TubeId = (FPGAId << 3) + LocalTube;
    uint32_t x = (TN - 1 - (TubeId / TZ)) * NStraws + StrawId + (TubeId % TZ) * TN * NStraws;
    uint32_t y = NPos - 1 - Pos;

    if (Vertical) {
      XTRACE(PROCESS, DEB, "Vert: tube %u straw %u pos %u - x %u y %u", TubeId, StrawId, Pos, x, y);
      return Geometry2D.pixel2D(x, y) + Offset;
    } else { // Rotated 90 degrees counter clock wise
      XTRACE(PROCESS, DEB, "Horiz: tube %u straw %u pos %u - x %u y %u", TubeId, StrawId, Pos, y, x);
      return Geometry2D.pixel2D(y, TZ*TN*NStraws - 1 - x) + Offset;
    }
  }

private:
  const uint8_t NStraws{7}; /// \todo not hardcode, should match HeliumTube.h
  const uint16_t NPos{512}; /// \todo not hardcode, should match HeliumTube.h

  ///< Initialised in constructor
  bool Vertical{true}; ///< Vertical is the canonical orientation
  uint8_t TZ{4}; ///< Tubes in the z-direction
  uint8_t TN{8}; ///< Tubes in the y/x direction depending on orientation
  uint32_t Offset{0};

  ///
  ESSGeometry Geometry2D;
};
} // namespace Loki
