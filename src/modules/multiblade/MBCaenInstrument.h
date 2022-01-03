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
#include <common/kafka/EV42Serializer.h>
#include <common/kafka/Producer.h>
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
bool parsePacket(char * data, int length, EV42Serializer & ev42ser);

  ///
  void ingestOneReadout(int cassette, const Readout & dp);

  ///
bool filterEvent(const Event & e);


// Two methods below from ref data test

// determine time gaps for clusters
void FixJumpsAndSort(int builder, std::vector<Readout> &vec);
// load and flush as appropriate
void LoadAndProcessReadouts(int builder, std::vector<Readout> &vec);

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

  DataParser parser;
  ESSGeometry essgeom;
  Config MultibladeConfig;
  std::shared_ptr<ReadoutFile> dumpfile;
};

} // namespace
