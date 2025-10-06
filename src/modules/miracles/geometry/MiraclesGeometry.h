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
#include <modules/caen/geometry/Config.h>
#include <modules/caen/geometry/Geometry.h>
#include <modules/caen/readout/DataParser.h>
#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {
class MiraclesGeometry : public Geometry {
public:
  explicit MiraclesGeometry(Config &CaenConfiguration);
  uint32_t calcPixel(DataParser::CaenReadout &Data) override;
  bool validateData(DataParser::CaenReadout &Data) override;

  /// \brief return local x-coordinate from amplitudes
  int xCoord(int Ring, int Tube, int AmpA, int AmpB);

  /// \brief return local y-coordinate from amplitudes
  int yCoord(int Ring, int AmpA, int AmpB);

  /// \brief return the position along the tube
  int posAlongUnit(int AmpA, int AmpB);

  /// \brief return the total number of serializers used by the geometry
  [[nodiscard]] inline size_t numSerializers() const override { return 1; }

  /// \brief calculate the serializer index for the given readout
  /// \param Data CaenReadout to calculate serializer index for
  [[nodiscard]] inline size_t
  calcSerializer(DataParser::CaenReadout &) const override {
    return 0;
  }

  /// \brief return the name of the serializer at the given index
  [[nodiscard]] inline std::string serializerName(size_t) const override {
    return "caen";
  }

  /// \brief return the tube number (0 or 1) for the given amplitudes
  /// \param AmpA amplitude from A-tube
  /// \param AmpB amplitude from B-tube
  /// \return 0 for A-tube, 1 for B-tube
  [[nodiscard]] inline int tubeAorB(int AmpA, int AmpB) {
    if (AmpA >= AmpB) {
      XTRACE(DATA, DEB, "A-tube (A: %f > B: %f)", AmpA, AmpB);
      return 0;
    } else {
      XTRACE(DATA, DEB, "B-tube (A: %f < B: %f)", AmpA, AmpB);
      return 1;
    }
  }
};

} // namespace Caen
