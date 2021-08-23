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

#include <common/EV42Serializer.h>
#include <common/Producer.h>
#include <common/monitor/Histogram.h>
#include <common/monitor/HistogramSerializer.h>
#include <logical_geometry/ESSGeometry.h>
#include <modules/readout/common/ReadoutParser.h>
#include <modules/readout/vmm3/VMM3Parser.h>

#include <freia/Counters.h>
#include <freia/geometry/Config.h>
#include <freia/FreiaBase.h> // to get MBSettings

namespace Freia {

class FreiaInstrument {
public:

  /// \brief 'create' the Freia instrument
  ///
  FreiaInstrument(Counters & counters, BaseSettings & EFUSettings, FreiaSettings & moduleSettings);

  /// \brief process parsed vmm data into events
  void processReadouts(void);

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
  /// \brief Stuff that 'ties' Freia together
  struct Counters & counters;
  FreiaSettings & ModuleSettings;

  ///
  uint16_t ncass;
  uint16_t nwires;
  uint16_t nstrips;

  std::string topic{""};
  std::string monitor{""};

  HistogramSerializer histfb{1, "freia"}; // reinit in ctor
  Hists histograms{1, 1}; // reinit in ctor
  // MBGeometry mbgeom{1, 1, 1}; // reinit in ctor
  // std::vector<EventBuilder> builders; // reinit in ctor

  ESSGeometry essgeom;

  // towards VMM3
  Config Conf;
  ReadoutParser ESSReadoutParser;
  VMM3Parser VMMParser;
};

} // namespace
