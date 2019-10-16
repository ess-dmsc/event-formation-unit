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
      essgeometry = ESSGeometry(NX*NS, NY, NZ, 1);
      XTRACE(PROCESS, DEB, "NX %u, NS %u, NZ %u, NY %u", NX, NS, NZ, NY);
    };

  /// tubeids from 0 - (NX - 1), strawids 0 - (NS - 1), ypos 0 - (NY - 1)
  uint32_t getPixelId(uint8_t tubeid, uint8_t strawid, uint16_t ypos) {
    if ((tubeid >= NX*NZ) or (strawid >= NS) or (ypos >= NY)) {
      XTRACE(PROCESS, WAR, "invalid encoding: tube %u straw %u pos %u",
              tubeid, strawid, ypos);
      return 0;
    }

    uint32_t x = (NX - 1 - (tubeid / 4)) * NS + strawid;
    uint32_t y = NY - 1 - ypos;
    uint32_t z = tubeid % 4;
    XTRACE(PROCESS, DEB, "tube %u straw %u pos %u - x %u y %u z %u",
            tubeid, strawid, ypos, x, y, z);
    return essgeometry.pixel3D(x, y, z);
  }

private:
  ESSGeometry essgeometry;
  uint16_t NX{0}; // x dimension
  uint16_t NY{0}; // y dimention
  uint16_t NZ{0}; // z dimension
  uint16_t NS{0}; // straws
};

} // namespace Loki
