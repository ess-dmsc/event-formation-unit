// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from tube and amplitudes
///
//===----------------------------------------------------------------------===//

#pragma once

#include "common/Statistics.h"
#include <common/debug/Trace.h>
#include <logical_geometry/ESSGeometry.h>
#include <modules/caen/geometry/Config.h>
#include <modules/caen/geometry/Geometry.h>
#include <modules/caen/readout/DataParser.h>
#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {
class MiraclesGeometry : public Geometry, ESSGeometry {
public:
  explicit MiraclesGeometry(Statistics &Stats, const Config &CaenConfiguration);
  bool validateReadoutData(const DataParser::CaenReadout &Data) override;

  /// \brief return local x-coordinate from amplitudes
  int xCoord(int Ring, int Tube, int AmpA, int AmpB);

  /// \brief return local y-coordinate from amplitudes
  int yCoord(int Ring, int AmpA, int AmpB);

  int tubeAorB(int AmpA, int AmpB);

  /// \brief return the position along the tube
  int posAlongUnit(int AmpA, int AmpB);

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
  // Per-detector resolution: number of pixels across one unit (tube pair)
  int GroupResolution;

protected:
  /// \brief Calculate pixel ID from readout data
  /// \param Data Pointer to CaenReadout object (cast internally)
  /// \return Calculated pixel ID, or 0 if calculation failed
  uint32_t calcPixelImpl(void *Data) override;
};
} // namespace Caen
