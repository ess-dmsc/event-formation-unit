// Copyright (C) 2019 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Caen Boron Coated Straw Tubes functions
///
/// Ref: Loki TG3.1 Detectors technology "Boron Coated Straw Tubes for LoKI"
/// Davide Raspino 04/09/2019
///
/// New terminology for charge division: Groups and Units rather than Tubes
/// and Straws
//===----------------------------------------------------------------------===//

#pragma once
#include <cinttypes>
#include <common/debug/Trace.h>
#include <logical_geometry/ESSGeometry.h>
#include <modules/caen/geometry/CDCalibration.h>
#include <modules/caen/geometry/Config.h>
#include <modules/caen/geometry/Geometry.h>
#include <modules/caen/readout/DataParser.h>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_ERR

namespace Caen {

class LokiGeometry : public Geometry, ESSGeometry {
public:
  explicit LokiGeometry(Statistics &Stats, Config &CaenConfiguration);

  /// \brief return the position along the tube
  /// \param AmpA amplitude A from readout data
  /// \param AmpB amplitude B from readout data
  /// \param AmpC amplitude C from readout data
  /// \param AmpD amplitude D from readout data
  /// \return tube index (0, 1, 2, .. 6) and normalised position [0.0 ; 1.0]
  /// or (-1, -1.0) if invalid
  std::pair<int, double> calcUnitAndPos(int Group, int AmpA, int AmpB, int AmpC,
                                        int AmpD) const;

  /// \brief The four amplitudes measured at certain points in the
  /// Helium tube circuit diagram are used to identify the straw that
  /// detected the neutron and also the position along the straw.
  /// Both of these are calculated at the same time and the result
  /// is stored in the two member variables (UnitId, PosId) if an
  /// invalid input is given the output will be outside the valid
  /// ranges.
  // bool calcPositions(int16_t AmplitudeA, int16_t AmplitudeB,
  //                    int16_t AmplitudeC, int16_t AmplitudeD);

  void setCalibration(const CDCalibration &Calib) { CaenCDCalibration = Calib; }

  bool validateReadoutData(const DataParser::CaenReadout &Data) const override;

  // Holds the parsed configuration
  Config &Conf;

  const std::pair<int, float> InvalidPos{-1, -1.0};

  // Per-detector resolution: number of pixels per unit/straw
  int StrawResolution;

  /// \brief return the total number of serializers used by the geometry
  [[nodiscard]] inline size_t numSerializers() const override { return 1; }

  /// \brief calculate the serializer index for the given readout
  /// \param Data CaenReadout to calculate serializer index for
  [[nodiscard]] inline size_t
  calcSerializer(const DataParser::CaenReadout &) const override {
    return 0;
  }

  /// \brief return the name of the serializer at the given index
  [[nodiscard]] inline std::string serializerName(size_t) const override {
    return "caen";
  }

protected:
  /// \brief Implementation for pixel calculation for Loki geometry
  /// \param Data Const reference to CaenReadout object
  /// \return Calculated pixel ID, or 0 if calculation failed
  uint32_t calcPixelImpl(const DataParser::CaenReadout &Data) const override;
};

} // namespace Caen
