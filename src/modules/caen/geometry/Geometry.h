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
#include <modules/caen/geometry/CDCalibration.h>
#include <modules/caen/readout/DataParser.h>

#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {
class Geometry {
public:
  /// \brief sets the pixel resolution of a straw
  /// \param Resolution integer value to set straw resolution to
  void setResolution(uint16_t Resolution) { NPos = Resolution; }

  /// \brief sets the calibration parameters for straw stretch corrections
  /// \param Calib Calibration object containing polynomial correction values
  void setCalibration(CDCalibration Calib) { CaenCDCalibration = Calib; }

  /// \brief calculates an integer pixel value from a CaenReadout object
  /// \param Data CaenReadout object, containing ADC value information,
  /// Group id and other information needed to determine pixel of
  /// event. If a Calibration has been set, it will be applied here.
  virtual uint32_t calcPixel(DataParser::CaenReadout &Data) = 0;

  /// \brief returns true if Data is a valid readout with the given config
  /// \param Data CaenReadout to check validity of.
  virtual bool validateData(DataParser::CaenReadout &Data) = 0;

  struct Stats {
    int64_t RingErrors{0};
    int64_t RingMappingErrors{0};
    int64_t FENErrors{0};
    int64_t FENMappingErrors{0};
    int64_t GroupErrors{0};
    int64_t AmplitudeZero{0};
  } Stats;

  CDCalibration CaenCDCalibration;
  ESSGeometry *ESSGeom;
  uint16_t NPos{512}; ///< resolution of position
  uint8_t MaxRing{2};
  uint8_t MaxFEN{0};
  uint8_t MaxGroup{14};
};
} // namespace Caen
