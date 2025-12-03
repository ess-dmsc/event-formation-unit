// Copyright (C) 2024 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Handle HEIMDAL geometry similarly as DREAM
///
/// Formulae created from ICD meeting on 2024-07-03 with CDT & friends
/// as of today no ICD exist
//===----------------------------------------------------------------------===//

#pragma once

#include <dream/geometry/Config.h>
#include <dream/geometry/Geometry.h>
#include <dream/geometry/HeimdalMantle.h>
#include <dream/readout/DataParser.h>

namespace Dream {

class HeimdalGeometry : public Geometry {
public:
  /// \brief Constructor
  /// \param Stats Reference to Statistics object for counter registration
  /// \param Config Reference to the DREAM configuration object
  HeimdalGeometry(Statistics &Stats, const Config &Config)
      : Geometry(Stats, Config) {}

  /// \brief return the global pixel id offset for each of the Heimdal detector
  /// components. This offset must be added to the local pixel id calculated
  /// for that module (see ICD for full description)
  int getPixelOffset(Config::ModuleType Type) const;

  /// \brief Implementation of pixel calculation using template specialization
  /// \param Data Const reference to CDTReadout object
  /// \return Calculated pixel ID, or 0 if calculation failed
  uint32_t calcPixelImpl(const DataParser::CDTReadout &Data) const override;

  HeimdalMantle mantle;
};
} // namespace Dream
