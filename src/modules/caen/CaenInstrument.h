// Copyright (C) 2020-2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating Caen processing from pipeline main loop
///
/// Holds efu stats, instrument readout mappings, logical geometry, pixel
/// calculations and LoKI readout parser
//===----------------------------------------------------------------------===//

#pragma once

#include <common/readout/ess/ESSTime.h>
#include <common/readout/ess/Parser.h>
#include <caen/Counters.h>
#include <caen/CaenBase.h> // to get CaenSettings
#include <caen/geometry/Calibration.h>
#include <caen/geometry/Config.h>
#include <caen/geometry/PanelGeometry.h>
#include <caen/geometry/TubeAmps.h>
#include <caen/readout/Readout.h>
#include <readout/DataParser.h>

namespace Caen {

class CaenInstrument {
public:
  /// \brief 'create' the LoKI instruments
  ///
  /// loads configuration and calibration files, calulate and generate the
  /// logical geometry and initialise the amplitude to position calculations
  CaenInstrument(Counters &counters, CaenSettings &moduleSettings);

  ~CaenInstrument();

  //
  void processReadouts();

  //
  void setSerializer(EV44Serializer *serializer) { Serializer = serializer; }
  void setSerializerII(EV44Serializer *serializer) { SerializerII = serializer; }

  /// \brief LoKI pixel calculations
  uint32_t calcPixel(PanelGeometry &Panel, uint8_t FEN,
                     DataParser::CaenReadout &Data);

  /// \brief writes a single readout to file
  void dumpReadoutToFile(DataParser::CaenReadout &Data);

public:
  /// \brief Stuff that 'ties' LoKI together
  struct Counters &counters;

  CaenSettings &ModuleSettings;
  Config CaenConfiguration;
  Calibration CaenCalibration;
  ESSReadout::Parser ESSReadoutParser;
  DataParser CaenParser{counters};
  TubeAmps Amp2Pos;
  EV44Serializer *Serializer;
  EV44Serializer *SerializerII;
  std::shared_ptr<ReadoutFile> DumpFile;
};

} // namespace Caen
