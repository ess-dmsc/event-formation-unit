// Copyright (C) 2023 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from tube and amplitudes
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/Statistics.h>
#include <common/debug/Trace.h>
#include <logical_geometry/ESSGeometry.h>
#include <modules/caen/geometry/Config.h>
#include <modules/caen/geometry/Geometry.h>
#include <string>
#include <vector>
//
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {
class CspecGeometry : public Geometry, ESSGeometry {
public:
  explicit CspecGeometry(Statistics &Stats, const Config &CaenConfiguration);

  bool validateReadoutData(const DataParser::CaenReadout &Data) const override;

  /// \brief return the global x-offset for the given identifiers
  int xOffset(int Ring, int Group) const;

  /// \brief return local y-coordinate from amplitudes
  int yCoord(int AmpA, int AmpB) const { return posAlongUnit(AmpA, AmpB); }

  /// \brief return the position along the unit (tube for CSPEC)
  int posAlongUnit(int AmpA, int AmpB) const;

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
  // Per-detector resolution: number of pixels across one unit
  int Resolution;

protected:
  /// \brief Calculate pixel ID from readout data for CSPEC geometry
  /// \param Data Const reference to CaenReadout object
  /// \return Calculated pixel ID, or 0 if calculation failed
  uint32_t calcPixelImpl(const DataParser::CaenReadout &Data) const override;
};
} // namespace Caen

// Per-detector resolution: number of pixels across one unit
// Note: CspecGeometry will use this value from its implementation file.
