// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CSPECInstrument is responsible for readout validation and event
/// formation
/// Its functions are called from the main prcessing loop in CSPECBase
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
#include <common/reduction/Event.h>
#include <cspec/Counters.h>
#include <cspec/CSPECBase.h>
#include <cspec/geometry/Config.h>
#include <cspec/geometry/CSPECGeometry.h>
#include <cspec/clustering/EventBuilder.h>

namespace Cspec {

class CSPECInstrument {
public:

  /// \brief 'create' the CSPEC instrument
  /// based on settings the constructor loads both configuration
  /// and calibration data. It then initialises event builders and
  /// histograms
  CSPECInstrument(Counters & counters,
                  CSPECSettings & moduleSettings,
                  EV42Serializer * serializer);

  /// \brief handle loading and application of configuration and calibration
  /// files. This step will throw an exception upon errors.
  void loadConfigAndCalib();

  /// \brief process parsed vmm data into clusters
  void processReadouts(void);

  /// \brief process clusters into events
  void generateEvents(std::vector<Event> & Events);

  /// \brief dump readout data to HDF5
  void dumpReadoutToFile(const ESSReadout::VMM3Parser::VMM3Data & Data);

  // \brief initialise the serializer. This is used both in CSPECInstrument
  // and CSPECBase. Called from CSPECBase
  void setSerializer(EV42Serializer *serializer) { Serializer = serializer; }

public:
  /// \brief Stuff that 'ties' CSPEC together
  struct Counters & counters;
  CSPECSettings & ModuleSettings;

  /// \brief serialiser (and producer) for events
  EV42Serializer *Serializer{nullptr};

  /// ADC value histograms for all channels
  Hists ADCHist{1, 1}; // reinit in ctor

  /// \brief One builder per cassette, rezise in constructor when we have
  /// parsed the configuration file and know the number of cassettes
  std::vector<EventBuilder> builders; // reinit in ctor

  /// \brief Instrument configuration (rings, FENs, Hybrids, etc)
  Config Conf;

  /// \brief logical geometry
  /// get pixel IDs from x- and y- coordinates
  ESSGeometry essgeom{432, 140, 16, 1};

  /// \brief digital geometry
  /// Defines which digital geometry to use
  /// for calculating pixel ids
  CSPECGeometry CSPECGeometryInstance;

  Geometry *GeometryInstance;

  // 
  std::vector<ESSReadout::Hybrid> Hybrids;

  /// \brief parser for the ESS Readout header
  ESSReadout::Parser ESSReadoutParser;

  /// \brief parser for VMM3 readout data
  ESSReadout::VMM3Parser VMMParser;

  /// \brief for dumping raw VMM3 readouts to HDF5 files
  std::shared_ptr<VMM3::ReadoutFile> DumpFile;
};

} // namespace
