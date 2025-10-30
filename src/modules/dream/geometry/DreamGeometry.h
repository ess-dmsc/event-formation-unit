// Copyright (C) 2022 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Attempt to handle the complex DREAM geometry which is assembled
/// from five different CDT module types with different characeteristics
///
/// Heavily uses the formulae and parameters from the DREAM ICD which can be
/// located through
/// https://confluence.esss.lu.se/display/ECDC/Instrument+Status+Overview
//===----------------------------------------------------------------------===//

#pragma once

#include <dream/geometry/Config.h>
#include <dream/geometry/Cuboid.h>
#include <dream/geometry/DreamMantle.h>
#include <dream/geometry/Geometry.h>
#include <dream/geometry/SUMO.h>
#include <dream/readout/DataParser.h>

namespace Dream {

class DreamGeometry : public Geometry {
public:
  /// \brief Constructor
  /// \param Stats Reference to Statistics object for counter registration
  /// \param Config Reference to the DREAM configuration object
  DreamGeometry(Statistics &Stats, const Config &Config)
      : Geometry(Stats, Config), fwec(Stats, 280, 256), bwec(Stats, 616, 256),
        mantle(Stats, 256), cuboid(Stats) {}

  /// \brief return the global pixel id offset for each of the DREAM detector
  /// components. This offset must be added to the local pixel id calculated
  /// for that module (see ICD for full description)
  int getPixelOffset(Config::ModuleType Type) const;

  /// \brief Validate readout data for DREAM geometry
  bool validateReadoutData(const DataParser::CDTReadout &Data) const override;

  /// \brief Implementation for pixel calculation for DREAM geometry
  /// \param Data Pointer to DreamReadout object (cast internally)
  /// \return Calculated pixel ID, or 0 if calculation failed
  uint32_t calcPixelImpl(const void *Data) const override;

  SUMO fwec;
  SUMO bwec;
  DreamMantle mantle;
  Cuboid cuboid;
};
} // namespace Dream
