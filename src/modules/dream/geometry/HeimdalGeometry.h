// Copyright (C) 2024 European Spallation Source, see LICENSE file
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
#include <dream/geometry/HeimdalMantle.h>
#include <dream/readout/DataParser.h>

namespace Dream {

class HeimdalGeometry {
public:
  /// \brief return the global pixel id offset for each of the Heimdal detector
  /// components. This offset must be added to the local pixel id calculated
  /// for that module (see ICD for full description)
  int getPixelOffset(Config::ModuleType Type);

  /// \brief return pixel id from the digital identifiers
  int getPixel(Config::ModuleParms &Parms, DataParser::DreamReadout &Data);

  HeimdalMantle mantle{64};
};
} // namespace Dream
