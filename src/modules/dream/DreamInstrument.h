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
private:
  struct Counters &Counters;
  BaseSettings &Settings;
  Config DreamConfiguration;
  DetectorType Type;
  DreamGeometry DreamGeom;
  MagicGeometry MagicGeom;
  HeimdalGeometry HeimdalGeom;
  EV44Serializer &Serializer;
  ESSReadout::Parser &ESSHeaderParser;

public:
  /// \brief 'create' the DREAM instrument
  ///
  /// loads configuration and calibration files, calculate and generate the
  /// logical geometry and initialise the amplitude to position calculations
  DreamInstrument(struct Counters &counters, BaseSettings &settings,
                  EV44Serializer &serializer,
                  ESSReadout::Parser &essHeaderParser);

  ~DreamInstrument();

  /// \brief process parsed vmm data into clusters
  void processReadouts();

  /// \brief get the a reference to the configuration of this instrument
  /// \return reference to the configuration
  Config &getConfiguration() { return DreamConfiguration; }

  /// \brief process clusters into events
  uint32_t calcPixel(Config::ModuleParms &Parms, DataParser::CDTReadout &Data);

  DataParser DreamParser{Counters};
};

} // namespace Dream
