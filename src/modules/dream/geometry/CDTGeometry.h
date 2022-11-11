// Copyright (C) 2022 European Spallation Source, see LICENSE file
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
#include <dream/geometry/Mantle.h>
#include <dream/geometry/SUMO.h>
#include <dream/readout/DataParser.h>

namespace Dream {

class CDTGeometry {
public:

  /// \brief return the global pixel id offset for each of the DREAM detector
  /// components. This offset must be added to the local pixel id calculated
  /// for that module (see ICD for full description)
  int getPixelOffset(Config::ModuleType Type);


  /// \brief return pixel id from the digital identifiers
  int getPixel(Config::ModuleParms & Parms, DataParser::DreamReadout & Data);


  SUMO sumo;
  Cuboid cuboid;
  Mantle mantle;
};
}
