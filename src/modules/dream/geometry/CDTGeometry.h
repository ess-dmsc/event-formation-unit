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

namespace Dream {

class CDTGeometry {
public:

  /// \brief return the global pixel id offset for each of the DREAM detector
  /// components. This offset must be added to the local pixel id calculated
  /// for that module (see ICD for full description)
  int getPixelOffset(Config::ModuleType Type);


  /// \brief return local pixel id from the digital identifiers
  int getLocalPixel(uint8_t Ring, uint8_t FEN, Config::ModuleType Type,
      uint8_t Cathode, uint8_t Anode);
};
}
