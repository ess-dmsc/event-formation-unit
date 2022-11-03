// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating DREAM processing from pipeline main loop
///
/// Holds efu stats, instrument readout mappings, logical geometry, pixel
/// calculations and DREAM readout parser
//===----------------------------------------------------------------------===//

#pragma once

#include <common/readout/ess/ESSTime.h>
#include <common/readout/ess/Parser.h>
#include <dream/Counters.h>
#include <dream/DreamBase.h> // to get DreamSettings
#include <dream/geometry/Config.h>
#include <dream/geometry/CDTGeometry.h>
#include <dream/readout/DataParser.h>

namespace Dream {

class DreamInstrument {
public:
  /// \brief 'create' the DREAM instrument
  ///
  /// loads configuration and calibration files, calulate and generate the
  /// logical geometry and initialise the amplitude to position calculations
  DreamInstrument(Counters &counters, DreamSettings &moduleSettings);

  ~DreamInstrument();

  //
  void processReadouts();

  //
  void setSerializer(EV42Serializer *serializer) { Serializer = serializer; }

  //
  uint32_t calcPixel(Config::ModuleParms & Parms, DataParser::DreamReadout & Data);

public:
  /// \brief Stuff that 'ties' DREAM together
  struct Counters &counters;
  DreamSettings &ModuleSettings;
  Config DreamConfiguration;
  ESSReadout::Parser ESSReadoutParser;
  DataParser DreamParser{counters};
  ESSReadout::ESSTime Time;
  EV42Serializer *Serializer;
  CDTGeometry Geometry;
};

} // namespace Dream
