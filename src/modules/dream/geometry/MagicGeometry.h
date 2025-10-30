// Copyright (C) 2023 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Handle MAGIC geometry similarly as DREAM
///
/// Uses the formulae and parameters from the MAGIC ICD which can be
/// located through
/// https://confluence.esss.lu.se/display/ECDC/Instrument+Status+Overview
//===----------------------------------------------------------------------===//

#pragma once

#include <dream/geometry/Config.h>
#include <dream/geometry/DreamMantle.h>
#include <dream/geometry/Geometry.h>
#include <dream/geometry/PADetector.h>
#include <dream/readout/DataParser.h>

namespace Dream {

class MagicGeometry : public Geometry {
public:
  /// \brief Constructor
  /// \param Stats Reference to Statistics object for counter registration
  /// \param Config Reference to the DREAM configuration object
  MagicGeometry(Statistics &Stats, const Config &Config)
      : Geometry(Stats, Config), padetector(Stats, 256, 512),
        frdetector(Stats, 128) {}

  /// \brief return the global pixel id offset for each of the DREAM detector
  /// components. This offset must be added to the local pixel id calculated
  /// for that module (see ICD for full description)
  int getPixelOffset(Config::ModuleType Type) const;

  /// \brief Implementation of pixel calculation through type-safe template
  uint32_t calcPixelImpl(const void *Data) const override;

  /// \brief Validate readout data for MAGIC geometry
  bool validateReadoutData(const DataParser::CDTReadout &Data) const override;

private:
  PADetector padetector;
  DreamMantle frdetector;
};
} // namespace Dream
