// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating Bifrost processing from pipeline main loop
///
/// Holds efu stats, instrument readout mappings, logical geometry, pixel
/// calculations and Bifrost readout parser
//===----------------------------------------------------------------------===//

#pragma once

#include <bifrost/Counters.h>
#include <bifrost/BifrostBase.h> // to get BifrostSettings
#include <bifrost/readout/Readout.h>
#include <common/readout/ess/Parser.h>
#include <common/readout/ess/ESSTime.h>
#include <readout/DataParser.h>


namespace Bifrost {

class BifrostInstrument {
public:
  /// \brief 'create' the LoKI instruments
  ///
  /// loads configuration and calibration files, calulate and generate the
  /// logical geometry and initialise the amplitude to position calculations
  BifrostInstrument(Counters &counters, BifrostSettings &moduleSettings);

  ~BifrostInstrument();

  //
  void processReadouts();

  //
  void setSerializer(EV42Serializer *serializer) { Serializer = serializer; }

  /// \brief Bifrost pixel calculations
  uint32_t calcPixel();

  /// \brief writes a single readout to file
  void dumpReadoutToFile(DataParser::BifrostReadout &Data);

public:
  /// \brief Stuff that 'ties' Bifrost together
  struct Counters &counters;

  BifrostSettings &ModuleSettings;
  //Config BifrostConfiguration;
  //Calibration BifrostCalibration;
  ESSReadout::Parser ESSReadoutParser;
  DataParser BifrostParser{counters};
  EV42Serializer *Serializer;
  std::shared_ptr<ReadoutFile> DumpFile;
};

} // namespace Bifrost
