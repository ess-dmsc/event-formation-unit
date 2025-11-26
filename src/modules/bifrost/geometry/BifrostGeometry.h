// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from tube and amplitudes
///
/// Pixel definitions taken from the ICD, the latest version of which
/// can be found through Instrument Status Overview
/// https://confluence.esss.lu.se/display/ECDC/Instrument+Status+Overview
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <logical_geometry/ESSGeometry.h>
#include <modules/caen/geometry/Config.h>
#include <modules/caen/geometry/Geometry.h>
#include <string>
#include <utility>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {
class BifrostGeometry : public Geometry, ESSGeometry {
public:
  // Detector geometry constants
  static constexpr int UNITS_PER_TRIPLETS{3};          ///< Tube triplets per group
  static constexpr int TRIPLETS_PER_RING{15};       ///< Triplets per ring
  static constexpr int UNIT_PIXELLATION{100}; ///< Pixels along tube
  static constexpr int ESSGEOMETRY_NX{900};         ///< X dimension (pixels)
  static constexpr int ESSGEOMETRY_NY{15};          ///< Y dimension (pixels)
  static constexpr int ESSGEOMETRY_NZ{1};           ///< Z dimension (pixels)
  static constexpr int ESSGEOMETRY_NP{1};           ///< P dimension (pixels)

  explicit BifrostGeometry(Statistics &Stats, Config &CaenConfiguration);

  ///\brief virtual method inherited from base class
  bool validateReadoutData(const DataParser::CaenReadout &Data) const override;

  /// \brief return the global x-offset for the given identifiers
  /// \param Ring logical ring as defined in the ICD
  /// \param Group - identifies a tube triplet (new chargediv nomenclature)
  int xOffset(int Ring, int Group) const;

  /// \brief return the global y-offset for the given identifiers
  /// \param Group - identifies a tube triplet (new chargediv nomenclature)
  int yOffset(int Group) const;

  /// \brief return the position along the tube
  /// \param AmpA amplitude A from readout data
  /// \param AmpB amplitude B from readout data
  /// \return tube index (0, 1, 2) and normalised position [0.0 ; 1.0]
  /// or (-1, -1.0) if invalid
  std::pair<int, double> calcUnitAndPos(int Group, int AmpA, int AmpB) const;

  [[nodiscard]] size_t numSerializers() const override;
  [[nodiscard]] size_t
  calcSerializer(const DataParser::CaenReadout &Data) const override;
  [[nodiscard]] std::string serializerName(size_t Index) const override;


  // Per-detector resolution: horizontal pixel stride used for ring offsets
  int StrideResolution;

  static constexpr std::pair<int, float> InvalidPos{-1, -1.0};

  // Holds the parsed configuration
  Config &Conf;

protected:
  /// \brief Implementation for pixel calculation for Bifrost geometry
  /// \param Data Const reference to CaenReadout object
  /// \return Calculated pixel ID, or 0 if calculation failed
  uint32_t calcPixelImpl(const DataParser::CaenReadout &Data) const override;
};
} // namespace Caen
