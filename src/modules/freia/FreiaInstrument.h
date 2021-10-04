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
#include <common/monitor/Histogram.h>
#include <common/monitor/HistogramSerializer.h>
#include <logical_geometry/ESSGeometry.h>
#include <common/readout/ess/Parser.h>
#include <common/readout/ess/ESSTime.h>
#include <common/readout/vmm3/Readout.h>
#include <common/readout/vmm3/VMM3Parser.h>
#include <common/readout/vmm3/Hybrid.h>
#include <multiblade/clustering/EventBuilder.h>

#include <freia/Counters.h>
#include <freia/geometry/Config.h>
#include <freia/geometry/Geometry.h>
#include <freia/FreiaBase.h> // to get MBSettings

namespace Freia {

class FreiaInstrument {
public:

  /// \brief 'create' the Freia instrument
  ///
  FreiaInstrument(Counters & counters,
                  /* BaseSettings & EFUSettings, */
                  FreiaSettings & moduleSettings,
                  EV42Serializer * serializer);

  /// \brief process parsed vmm data into clusters
  void processReadouts(void);

  /// \brief process clusters into events
  void generateEvents(std::vector<Event> & Events);

  /// \brief dump readout data to HDF5
  void dumpReadoutToFile(const ESSReadout::VMM3Parser::VMM3Data & Data);

  //
  void setSerializer(EV42Serializer *serializer) { Serializer = serializer; }

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

  /// \brief serialiser (and producer) for events
  EV42Serializer *Serializer{nullptr};

  HistogramSerializer histfb{1, "freia"}; // reinit in ctor
  Hists histograms{1, 1}; // reinit in ctor

  /// \brief One builder per cassette, rezise in constructor when we have
  /// parsed the configuration file and know the number of cassettes
  std::vector<Multiblade::EventBuilder> builders; // reinit in ctor

  /// \brief Instrument configuration (rings, cassettes, FENs)
  Config Conf;

  /// \brief digital geometry
  /// get x- and y- coordinates from cassettes and channels
  Geometry FreiaGeom;

  /// \brief logical geometry
  /// get pixel IDs from x- and y- coordinates
  ESSGeometry essgeom{64, 1024, 1, 1};

  // Each cassette holds 2 VMMCalibrations
  std::vector<ESSReadout::Hybrid> Hybrids;

  /// \brief parser for the ESS Readout header
  ESSReadout::Parser ESSReadoutParser;

  /// \brief parser for VMM3 readout data
  ESSReadout::VMM3Parser VMMParser;

  /// \brief for dumping raw VMM3 readouts to HDF5 files
  std::shared_ptr<VMM3::ReadoutFile> DumpFile;

  const uint16_t TimeBoxNs{2010}; ///\todo add to config
};

} // namespace
