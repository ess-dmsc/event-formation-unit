// Copyright (C) 2020 - 2021 European Spallation Source, ERIC. See LICENSE file
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
#include <modules/readout/common/ReadoutParser.h>
#include <modules/readout/vmm3/VMM3Parser.h>
#include <caen/Config.h>


#include <multiblade/Counters.h>
#include <multiblade/MBCaenBase.h> // to get MBSettings

namespace Multiblade {

class MBCaenInstrument {
public:

  /// \brief 'create' the Multiblade instrument
  ///
  MBCaenInstrument(Counters & counters, BaseSettings & EFUSettings, CAENSettings & moduleSettings);

  ///
  //void ingestOneReadout(int cassette, const Readout & dp);

  ///
  //bool filterEvent(const Event & e);


  // Two methods below from ref data test

  // determine time gaps for clusters
  //void FixJumpsAndSort(int builder, std::vector<Readout> &vec);
  // load and flush as appropriate
  //void LoadAndProcessReadouts(int builder, std::vector<Readout> &vec);

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
  std::vector<EventBuilder> builders; // reinit in ctor

  ESSGeometry essgeom;
  Config MultibladeConfig;

  // towards VMM3
  ReadoutParser ESSReadoutParser;
  VMM3Parser VMMParser{counters};
};

} // namespace
