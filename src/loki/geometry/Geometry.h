/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Digital and logical geometry for Loki
///
/// Works directly on (fpga), tubeid, strawid and ypos values calcuated
/// from the Loki four-amplitude readout.
/// all ids are enumerated starting from 0. NOTE THIS DIFFERS FROM the
/// Loki documentation where enumerations begin at 1
/// Ref: Loki TG3.1 Detectors technology "Boron Coated Straw Tubes for LoKI"
/// Davide Raspino 04/09/2019
//===----------------------------------------------------------------------===//

#pragma once
#include <common/Trace.h>
#include <logical_geometry/ESSGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Loki {

/// \brief geometry class for Loki
class Geometry {
public:

  /// \brief geometry of a single nx x nz array of tubes
  /// \param nxTubes number of tubes in the x-direction
  /// \param nzTubes number of tubes in the z-direction
  /// \param nStraws number of straws per tube
  /// \param resolution number of coordinates in the y-direction
  Geometry(uint16_t nxTubes, uint16_t nzTubes, uint16_t nStraws, uint16_t resolution)
    : NX(nxTubes), NY(resolution), NZ(nzTubes), NS(nStraws){
      LokiGeometry = ESSGeometry(NX*NS, NY, NZ, 1);
      LokiGeometry2D = ESSGeometry(NX*NS*NZ, NY, 1, 1);
      XTRACE(PROCESS, DEB, "NX %u, NS %u, NZ %u, NY %u", NX, NS, NZ, NY);
    };

  /// \brief convert (tube, straw, pos) to pixel using ESSGeometry
  /// \param tubeid tubeid runs from 0 - (NX*NZ - 1)
  /// \param strawid runs from 0 - (NS - 1)
  /// \param ypos runs from 0 - (NY - 1)
  /// \return 0 upon error, else Single Panel 3D PixelId
  uint32_t getPixelId3D(uint8_t TubeId, uint8_t StrawId, uint16_t YPos) {
    if ((TubeId >= NX*NZ) or (StrawId >= NS) or (YPos >= NY)) {
      XTRACE(PROCESS, WAR, "invalid encoding: tube %u straw %u pos %u",
              TubeId, StrawId, YPos);
      return 0;
    }

    uint32_t x = (NX - 1 - (TubeId / 4)) * NS + StrawId;
    uint32_t y = NY - 1 - YPos;
    uint32_t z = TubeId % 4;
    XTRACE(PROCESS, DEB, "tube %u straw %u pos %u - x %u y %u z %u",
            TubeId, StrawId, YPos, x, y, z);
    return LokiGeometry.pixel3D(x, y, z);
  }

  /// \brief convert (tube, straw, pos) to pixel using ESSGeometry
  /// \param tubeid tubeid runs from 0 - (NX*NZ - 1)
  /// \param strawid runs from 0 - (NS - 1)
  /// \param ypos runs from 0 - (NY - 1)
  /// \return 0 upon error, else Single Panel 2D PixelId
  uint32_t getPixelId2D(uint8_t TubeId, uint8_t StrawId, uint16_t YPos) {
    if ((TubeId >= NX*NZ) or (StrawId >= NS) or (YPos >= NY)) {
      XTRACE(PROCESS, WAR, "invalid encoding: tube %u straw %u pos %u",
              TubeId, StrawId, YPos);
      return 0;
    }

    uint32_t x = (NX - 1 - (TubeId / 4)) * NS + StrawId + (TubeId % 4) * NX * NS;
    uint32_t y = NY - 1 - YPos;
    XTRACE(PROCESS, DEB, "tube %u straw %u pos %u - x %u y %u",
            TubeId, StrawId, YPos, x, y);
    return LokiGeometry2D.pixel2D(x, y);
  }

private:
  ESSGeometry LokiGeometry;
  ESSGeometry LokiGeometry2D;
  uint16_t NX{0}; // x dimension
  uint16_t NY{0}; // y dimention
  uint16_t NZ{0}; // z dimension
  uint16_t NS{0}; // straws
};
} // namespace Loki
