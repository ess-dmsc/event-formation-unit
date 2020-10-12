// Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating Loki processing from pipeline main loop
///
/// Holds efu stats, instrument readout mappings, logical geometry, pixel
/// calculations and LoKI readout parser
//===----------------------------------------------------------------------===//

#pragma once

#include <loki/Counters.h>
#include <loki/LokiBase.h> // to get LokiSettings
#include <modules/readout/ReadoutParser.h>
#include <readout/DataParser.h>


namespace Loki {

class LokiInstrument {
public:

  /// \brief 'create' the LoKI instruments
  ///
  /// loads configuration and calibration files, calulate and generate the
  /// logical geometry and initialise the amplitude to position calculations
  LokiInstrument(Counters & counters, LokiSettings & moduleSettings);


  /// \brief LoKI pixel calculations
  uint32_t calcPixel(PanelGeometry & Panel, uint8_t FEN,
                     DataParser::LokiReadout & Data);


  /// \brief writes a single readout to file
  void dumpReadoutToFile(DataParser::ParsedData & Section, DataParser::LokiReadout & Data);

public:
  /// \brief Stuff that 'ties' LoKI together
  struct Counters & counters;
  LokiSettings & ModuleSettings;
  Config LokiConfiguration;
  Calibration LokiCalibration;
  ReadoutParser ESSReadoutParser;
  DataParser LokiParser{counters};
  TubeAmps Amp2Pos;
  // ESSTime Time;
  std::shared_ptr<ReadoutFile> DumpFile;
};

} // namespace
