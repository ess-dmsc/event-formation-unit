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

#include <dream/Counters.h>
#include <dream/DreamBase.h> // to get DreamSettings
#include <dream/readout/DataParser.h>
#include <modules/readout/ReadoutParser.h>
#include <modules/readout/ESSTime.h>


namespace Jalousie {

class DreamInstrument {
public:

  /// \brief 'create' the DREAM instrument
  ///
  /// loads configuration and calibration files, calulate and generate the
  /// logical geometry and initialise the amplitude to position calculations
  DreamInstrument(Counters & counters, DreamSettings & moduleSettings);


  ~DreamInstrument();

  //
  void processReadouts();


  //
  void setSerializer(EV42Serializer * serializer) { Serializer = serializer; }

  //
  uint32_t calcPixel(uint8_t Sector, uint8_t Sumo, uint8_t Strip,
                     uint8_t Wire, uint8_t Cassette, uint8_t Counter);

public:
  /// \brief Stuff that 'ties' DREAM together
  struct Counters & counters;
  DreamSettings & ModuleSettings;
  ReadoutParser ESSReadoutParser;
  DataParser DreamParser{counters};
  ESSTime Time;
  EV42Serializer * Serializer;
};

} // namespace
