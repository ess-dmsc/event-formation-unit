/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Digital and logical geometry for Loki
///
//===----------------------------------------------------------------------===//

#pragma once
//#include <loki/readout/Readout.h>

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
    : NX(nxTubes), NY(resolution), NZ(nzTubes), NS(nStraws){};

  /// \brief return the straw index from the four amplitudes
  uint8_t getStrawId(uint16_t __attribute__((unused)) A, uint16_t __attribute__((unused)) B,
  uint16_t __attribute__((unused)) C, uint16_t __attribute__((unused)) D) {
    return 0;
  }

  /// \brief return pixel id for the readout
  uint32_t getPixelId() {
    return 0;
  }

private:
  uint16_t __attribute__((unused)) NX{0}; // x dimension
  uint16_t __attribute__((unused)) NY{0}; // y dimention
  uint16_t __attribute__((unused)) NZ{0}; // z dimension
  uint16_t __attribute__((unused)) NS{0}; // straws
};

} // namespace Loki
