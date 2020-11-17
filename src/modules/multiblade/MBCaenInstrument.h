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
#include <common/monitor/Histogram.h>
#include <logical_geometry/ESSGeometry.h>
#include <caen/Config.h>
#include <multiblade/Counters.h>
#include <multiblade/MBCaenBase.h> // to get MBSettings

namespace Multiblade {

class MBCaenInstrument {
public:

  /// \brief 'create' the Multiblade instrument
  ///
  MBCaenInstrument(Counters & counters, CAENSettings & moduleSettings);

  ///
  void ingestOneReadout(int cassette, const Readout & dp);

public:
  /// \brief Stuff that 'ties' Multiblade together
  struct Counters & counters;
  CAENSettings & ModuleSettings;

  ///
  uint16_t ncass;
  uint16_t nwires;
  uint16_t nstrips;

  Hists histograms{1, 1}; // will be reinitialised in constructor
  std::vector<EventBuilder> builders;
  MBGeometry mbgeom{1, 1, 1}; // will be reinitialised in constructor
  ESSGeometry essgeom;
  Config MultibladeConfig;

  std::string topic{""};
  std::string monitor{""};
};

} // namespace
