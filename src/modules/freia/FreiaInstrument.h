// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief FreiaInstrument is responsible for readout validation and event
/// formation
/// Its functions are called from the main prcessing loop in FreiaBase
//===----------------------------------------------------------------------===//

#pragma once

#include <common/kafka/EV42Serializer.h>
#include <common/monitor/Histogram.h>
#include <logical_geometry/ESSGeometry.h>
#include <common/readout/ess/Parser.h>
#include <common/readout/ess/ESSTime.h>
#include <common/readout/vmm3/Readout.h>
#include <common/readout/vmm3/VMM3Parser.h>
#include <common/readout/vmm3/Hybrid.h>
#include <freia/clustering/EventBuilder.h>
#include <freia/Counters.h>
#include <freia/geometry/Config.h>
#include <freia/geometry/Geometry.h>
#include <freia/FreiaBase.h>

namespace Freia {

class FreiaInstrument {
public:

  /// \brief 'create' the Freia instrument
  /// based on settings the constructor loads both configuration
  /// and calibration data. It then initialises event builders and
  /// histograms
  FreiaInstrument(Counters & counters,
                  FreiaSettings & moduleSettings,
                  EV42Serializer * serializer);

  /// \brief handle loading and application of configuration and calibration
  /// files. This step will throw an exception upon errors.
  void loadConfigAndCalib();

  /// \brief after loading the config file, Config.Parms.HybridIdStr contains
  /// a vector of HybridIds. These are then loaded into the Hybrids so that
  /// we can later do consistency checks when applying the calibration data
  void setHybridIds(std::vector<std::string> Ids);

  /// \brief process parsed vmm data into clusters
  void processReadouts(void);

  /// \brief process vmm-formatted monitor readouts
  void processMonitorReadouts(void);

  /// \brief process clusters into events
  void generateEvents(std::vector<Event> & Events);

  /// \brief dump readout data to HDF5
  void dumpReadoutToFile(const ESSReadout::VMM3Parser::VMM3Data & Data);

  // \brief initialise the serializer. This is used both in FreiaInstrument
  // and FreiaBase. Called from FreiaBase
  void setSerializer(EV42Serializer *serializer) { Serializer = serializer; }

public:
  /// \brief Stuff that 'ties' Freia together
  struct Counters & counters;
  FreiaSettings & ModuleSettings;

  /// \brief serialiser (and producer) for events
  EV42Serializer *Serializer{nullptr};

  /// ADC value histograms for all channels
  Hists ADCHist{1, 1}; // reinit in ctor
  Hists TDCHist{1, 1}; // reinit in ctor

  /// \brief One builder per cassette, rezise in constructor when we have
  /// parsed the configuration file and know the number of cassettes
  std::vector<EventBuilder> builders; // reinit in ctor

  /// \brief Instrument configuration (rings, cassettes, FENs)
  Config Conf;

  /// \brief digital geometry
  /// get x- and y- coordinates from cassettes and channels
  Geometry Geom;

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
};

} // namespace
