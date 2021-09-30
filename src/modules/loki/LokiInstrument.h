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
#include <loki/geometry/Calibration.h>
#include <loki/geometry/Config.h>
#include <loki/geometry/PanelGeometry.h>
#include <loki/geometry/TubeAmps.h>
#include <loki/readout/Readout.h>
#include <modules/readout/common/ReadoutParser.h>
#include <modules/readout/common/ESSTime.h>
#include <readout/DataParser.h>


namespace Loki {

class LokiInstrument {
public:
  /// \brief 'create' the LoKI instruments
  ///
  /// loads configuration and calibration files, calulate and generate the
  /// logical geometry and initialise the amplitude to position calculations
  LokiInstrument(Counters &counters, LokiSettings &moduleSettings);

  ~LokiInstrument();

  //
  void processReadouts();

  //
  void setSerializer(EV42Serializer *serializer) { Serializer = serializer; }
  void setSerializerII(EV42Serializer *serializer) { SerializerII = serializer; }

  /// \brief LoKI pixel calculations
  uint32_t calcPixel(PanelGeometry &Panel, uint8_t FEN,
                     DataParser::LokiReadout &Data);

  /// \brief writes a single readout to file
  void dumpReadoutToFile(DataParser::ParsedData &Section,
                         DataParser::LokiReadout &Data);

public:
  /// \brief Stuff that 'ties' LoKI together
  struct Counters &counters;

  LokiSettings &ModuleSettings;
  Config LokiConfiguration;
  Calibration LokiCalibration;
  ESSReadout::ReadoutParser ESSReadoutParser;
  DataParser LokiParser{counters};
  TubeAmps Amp2Pos;
  EV42Serializer *Serializer;
  EV42Serializer *SerializerII;
  std::shared_ptr<ReadoutFile> DumpFile;
};

} // namespace Loki
