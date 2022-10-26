// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from tube and amplitudes
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <logical_geometry/ESSGeometry.h>
#include <modules/caen/geometry/Calibration.h>
#include <modules/caen/readout/DataParser.h>

#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {
class Geometry {
public:

  void setResolution(uint16_t Resolution) { NPos = Resolution; }
  void setCalibration(Calibration Calib) { CaenCalibration = Calib; }

  virtual uint32_t calcPixel(DataParser::CaenReadout &Data) = 0;  
  virtual bool validateData(DataParser::CaenReadout &Data) = 0;

  struct Stats {
    uint64_t AmplitudeZero{0};
    uint64_t OutsideRegion{0};
    uint64_t RingErrors{0};
    uint64_t FENErrors{0};
  } Stats;


  Calibration CaenCalibration;
  ESSGeometry *ESSGeom;
  std::uint16_t NPos{512};       ///< resolution of position
  uint8_t MaxRing{0};
};
} // namespace Caen
