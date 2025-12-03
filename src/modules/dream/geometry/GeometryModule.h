// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Common interface for DREAM geometry modules
///
/// This interface allows DreamGeometry to register and manage different
/// detector module types (SUMO, Mantle, Cuboid) in a unified way, only
/// instantiating modules that are actually used by the configuration.
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>
#include <dream/geometry/Config.h>
#include <dream/readout/DataParser.h>

namespace Dream {

/// \brief Abstract base class for all DREAM geometry modules
///
/// Provides a common interface for calculating pixel IDs from readout data.
/// Each specific module type (SUMO, Mantle, Cuboid) implements this interface.
class DreamSubModule {
public:
  virtual ~DreamSubModule() = default;

  /// \brief Calculate pixel ID from CDT readout data
  /// \param Parms Module parameters from configuration
  /// \param Data CDT readout data
  /// \return Local pixel ID (before offset), or 0 on error
  virtual uint32_t calcPixelId(const Config::ModuleParms &Parms,
                                const DataParser::CDTReadout &Data) const = 0;
};

} // namespace Dream
