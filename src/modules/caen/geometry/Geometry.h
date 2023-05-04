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
  /// \brief sets the pixel resolution of a straw
  /// \param Resolution integer value to set straw resolution to
  void setResolution(uint16_t Resolution) { NPos = Resolution; }

  /// \brief sets the calibration parameters for straw stretch corrections
  /// \param Calib Calibration object containing polynomial correction values
  void setCalibration(Calibration Calib) { CaenCalibration = Calib; }

  /// \brief calculates an integer pixel value from a CaenReadout object
  /// \param Data CaenReadout object, containing ADC value information,
  ///         TubeID, and other information needed to determine pixel of
  ///         event. If a Calibration has been set, it will be applied here.
  virtual uint32_t calcPixel(DataParser::CaenReadout &Data) = 0;

  /// \brief returns true if Data is a valid readout with the given config
  /// \param Data CaenReadout to check validity of.
  virtual bool validateData(DataParser::CaenReadout &Data) = 0;

  struct Stats {
    int64_t *RingErrors;
    int64_t *FENErrors;
    int64_t *TubeErrors;
    int64_t *AmplitudeZero;
    int64_t *OutsideTube;
    int64_t *CalibrationErrors;
  } Stats;

  Calibration CaenCalibration;
  ESSGeometry *ESSGeom;
  std::uint16_t NPos{512}; ///< resolution of position
  uint8_t MaxRing{2};
  uint8_t MaxFEN{0};
  uint8_t MaxTube{14};
};
} // namespace Caen
