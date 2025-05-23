// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
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

#include <common/readout/ess/Parser.h>
#include <dream/Counters.h>
#include <dream/DreamBase.h> // to get DreamSettings
#include <dream/geometry/Config.h>
#include <dream/geometry/DreamGeometry.h>
#include <dream/geometry/HeimdalGeometry.h>
#include <dream/geometry/MagicGeometry.h>
#include <dream/readout/DataParser.h>

namespace Dream {

class DreamInstrument {
public:
  /// \brief 'create' the DREAM instrument
  ///
  /// loads configuration and calibration files, calculate and generate the
  /// logical geometry and initialise the amplitude to position calculations
  DreamInstrument(Counters &counters, BaseSettings &settings);

  ~DreamInstrument();

  //
  void processReadouts();

  //
  void setSerializer(EV44Serializer *serializer) { Serializer = serializer; }

  //
  uint32_t calcPixel(Config::ModuleParms &Parms,
                     DataParser::CDTReadout &Data);

public:
  /// \brief Stuff that 'ties' DREAM together
  struct Counters &counters;
  BaseSettings &Settings;
  Config DreamConfiguration;
  ESSReadout::Parser ESSReadoutParser;
  DataParser DreamParser{counters};
  ESSReadout::ESSReferenceTime Time;
  DetectorType Type;
  EV44Serializer *Serializer;
  DreamGeometry DreamGeom;
  MagicGeometry MagicGeom;
  HeimdalGeometry HeimdalGeom;
};

} // namespace Dream
