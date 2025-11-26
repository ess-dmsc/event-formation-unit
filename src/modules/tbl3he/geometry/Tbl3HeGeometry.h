// Copyright (C) 2024 - 2025 European Spallation Source, ERIC. See LICENSE file
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

#include <common/Statistics.h>
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
class Tbl3HeGeometry : public Geometry, ESSGeometry {
public:
  Tbl3HeGeometry(Statistics &Stats, const Config &CaenConfiguration);

  ///\brief virtual method inherited from base class
  bool validateReadoutData(const DataParser::CaenReadout &Data) const override;

  /// \brief return the position along the tube
  /// \param AmpA amplitude A from readout data
  /// \param AmpB amplitude B from readout data
  /// \return tube index (0) and normalised position [0.0 ; 1.0]
  /// or (-1, -1.0) if invalid
  /// \todo refactoring oportunity: their code is the same as for bifrost
  std::pair<int, double> calcUnitAndPos(int Group, int AmpA, int AmpB) const;

  /// \todo functions to handle multiple serialisers
  [[nodiscard]] size_t numSerializers() const override;
  [[nodiscard]] size_t
  calcSerializer(const DataParser::CaenReadout &Data) const override;
  [[nodiscard]] std::string serializerName(size_t Index) const override;

  const int UnitsPerGroup{1};

  const std::pair<int, float> InvalidPos{-1, -1.0};

protected:
  /// \brief Calculate pixel ID from readout data for Tbl3He geometry
  /// \param Data Const reference to CaenReadout object
  /// \return Calculated pixel ID, or 0 if calculation failed
  uint32_t calcPixelImpl(const DataParser::CaenReadout &Data) const override;

private:
  const Config &Conf;

  int UnitPixellation{100}; ///< Number of pixels along a single He tube.
};
} // namespace Caen
