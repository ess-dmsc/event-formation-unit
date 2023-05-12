// Copyright (C) 2023 European Spallation Source, see LICENSE file
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
#include <dream/geometry/DetB.h>
#include <dream/geometry/Mantle.h>
#include <dream/readout/DataParser.h>

namespace Dream {

class MagicGeometry {
public:
  /// \brief return the global pixel id offset for each of the DREAM detector
  /// components. This offset must be added to the local pixel id calculated
  /// for that module (see ICD for full description)
  int getPixelOffset(Config::ModuleType Type);

  /// \brief return pixel id from the digital identifiers
  int getPixel(Config::ModuleParms &Parms, DataParser::DreamReadout &Data);

  DetB magicb{256, 512};
  Mantle mantle{128};
};
} // namespace Dream
