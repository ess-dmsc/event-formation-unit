// Copyright (C) 2020-2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating Caen processing from pipeline main loop
///
/// Holds efu stats, instrument readout mappings, logical geometry, pixel
/// calculations and Caen readout parser
//===----------------------------------------------------------------------===//

#pragma once

#include <bifrost/geometry/BifrostGeometry.h>
#include <caen/CaenBase.h> // to get CaenSettings
#include <caen/Counters.h>
#include <caen/geometry/Calibration.h>
#include <caen/geometry/Config.h>
#include <caen/readout/Readout.h>
#include <common/readout/ess/ESSTime.h>
#include <common/readout/ess/Parser.h>
#include <loki/geometry/LokiGeometry.h>
#include <loki/geometry/PanelGeometry.h>
#include <miracles/geometry/MiraclesGeometry.h>
#include <readout/DataParser.h>

namespace Caen {

class CaenInstrument {
public:
  /// \brief 'create' the Caen instruments
  ///
  /// loads configuration and calibration files, calulate and generate the
  /// logical geometry and initialise the amplitude to position calculations
  CaenInstrument(Counters &counters, BaseSettings &settings);

  ~CaenInstrument();

  //
  void processReadouts();

  //
  void setSerializer(EV42Serializer *serializer) { Serializer = serializer; }
  void setSerializerII(EV42Serializer *serializer) {
    SerializerII = serializer;
  }

  /// \brief Caen pixel calculations
  uint32_t calcPixel(DataParser::CaenReadout &Data);

  /// \brief writes a single readout to file
  void dumpReadoutToFile(DataParser::CaenReadout &Data);

public:
  /// \brief Stuff that 'ties' Caen together
  struct Counters &counters;

  Config CaenConfiguration;
  BaseSettings &Settings;
  ESSReadout::Parser ESSReadoutParser;
  DataParser CaenParser{counters};
  Geometry *Geom;
  EV42Serializer *Serializer;
  EV42Serializer *SerializerII;
  std::shared_ptr<ReadoutFile> DumpFile;
};

} // namespace Caen
