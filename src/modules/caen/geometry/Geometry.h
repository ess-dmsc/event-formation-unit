// Copyright (C) 2022 - 2024 European Spallation Source, ERIC. See LICENSE file
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

  /// \brief return the total number of serializers used by the geometry
  [[nodiscard]] virtual size_t numSerializers() const = 0;

  /// \brief calculate the serializer index for the given readout
  /// \param Data CaenReadout to calculate serializer index for
  [[nodiscard]] virtual size_t calcSerializer(DataParser::CaenReadout &Data) const = 0;

  /// \brief return the name of the serializer at the given index
  [[nodiscard]] virtual std::string serializerName(size_t Index) const = 0;

  struct Stats {
    int64_t RingErrors{0};
    int64_t RingMappingErrors{0};
    int64_t FENErrors{0};
    int64_t FENMappingErrors{0};
    int64_t TopologyErrors{0}; ///\todo replace mapping errors?
    int64_t GroupErrors{0};
    int64_t AmplitudeZero{0};
    int64_t AmplitudeLow{0};
    int64_t AmplitudeHigh{0};
  } Stats;

  CDCalibration CaenCDCalibration;
  ESSGeometry *ESSGeom;
  uint16_t NPos{512}; ///< resolution of position
  uint8_t MaxRing{0};
  uint8_t MaxFEN{0};
  uint8_t MaxGroup{0};
  int MinAmpl{0};
  int MaxAmpl{(std::numeric_limits<int>::max)()};
};
} // namespace Caen
