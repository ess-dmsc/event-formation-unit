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

#include <common/readout/ess/ESSTime.h>
#include <common/readout/ess/Parser.h>
#include <loki/Counters.h>
#include <loki/LokiBase.h> // to get LokiSettings
#include <loki/geometry/Calibration.h>
#include <loki/geometry/Config.h>
#include <loki/geometry/PanelGeometry.h>
#include <loki/geometry/TubeAmps.h>
#include <loki/readout/Readout.h>
#include <readout/DataParser.h>

namespace Loki {

class LokiInstrument {
public:
  /// \brief 'create' the LoKI instruments
  ///
  /// loads configuration and calibration files, calulate and generate the
  /// logical geometry and initialise the amplitude to position calculations
  LokiInstrument(Counters &counters, BaseSettings &settings);

  ~LokiInstrument();

  //
  void processReadouts();

  //
  void setSerializer(EV44Serializer *serializer) { Serializer = serializer; }
  void setSerializerII(EV44Serializer *serializer) { SerializerII = serializer; }

  /// \brief LoKI pixel calculations
  uint32_t calcPixel(PanelGeometry &Panel, uint8_t FEN,
                     DataParser::LokiReadout &Data);

  /// \brief writes a single readout to file
  void dumpReadoutToFile(DataParser::LokiReadout &Data);

public:
  /// \brief Stuff that 'ties' LoKI together
  struct Counters &counters;

  BaseSettings &Settings;
  Config LokiConfiguration;
  Calibration LokiCalibration;
  ESSReadout::Parser ESSReadoutParser;
  DataParser LokiParser{counters};
  TubeAmps Amp2Pos;
  EV44Serializer *Serializer;
  EV44Serializer *SerializerII;
  std::shared_ptr<ReadoutFile> DumpFile;
};

} // namespace Loki
