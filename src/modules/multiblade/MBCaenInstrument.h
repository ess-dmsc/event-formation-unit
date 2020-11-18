// Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating MB processing from pipeline main loop
///
/// Holds efu stats, ...
//===----------------------------------------------------------------------===//

#pragma once

#include <caen/MBGeometry.h>
#include <clustering/EventBuilder.h>
#include <common/EV42Serializer.h>
#include <common/Producer.h>
#include <common/monitor/Histogram.h>
#include <common/monitor/HistogramSerializer.h>
#include <logical_geometry/ESSGeometry.h>
#include <caen/Config.h>
#include <caen/DataParser.h>

#include <multiblade/Counters.h>
#include <multiblade/MBCaenBase.h> // to get MBSettings

namespace Multiblade {

class MBCaenInstrument {
public:

  /// \brief 'create' the Multiblade instrument
  ///
  MBCaenInstrument(Counters & counters, BaseSettings & EFUSettings, CAENSettings & moduleSettings);

  ///
  void parsePacket(char * data, int length);

  ///
  void ingestOneReadout(int cassette, const Readout & dp);

  ///
bool filterEvent(const Event & e);

public:
  /// \brief Stuff that 'ties' Multiblade together
  struct Counters & counters;
  CAENSettings & ModuleSettings;

  ///
  uint16_t ncass;
  uint16_t nwires;
  uint16_t nstrips;

  std::string topic{""};
  std::string monitor{""};

  HistogramSerializer histfb{1, "multiblade"}; // reinit in ctor
  Hists histograms{1, 1}; // reinit in ctor
  MBGeometry mbgeom{1, 1, 1}; // reinit in ctor

  std::vector<EventBuilder> builders;
  DataParser parser;
  ESSGeometry essgeom;
  Config MultibladeConfig;
};

} // namespace
