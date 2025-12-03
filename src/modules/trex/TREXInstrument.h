// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief TREXInstrument is responsible for readout validation and event
/// formation
/// Its functions are called from the main processing loop in TREXBase
//===----------------------------------------------------------------------===//

#pragma once

#include <common/kafka/EV44Serializer.h>
#include <common/monitor/Histogram.h>
#include <common/readout/ess/Parser.h>
#include <common/readout/vmm3/Hybrid.h>
#include <common/readout/vmm3/Readout.h>
#include <common/readout/vmm3/VMM3Parser.h>
#include <common/reduction/Event.h>
#include <common/reduction/EventBuilder2D.h>
#include <logical_geometry/ESSGeometry.h>
#include <trex/Counters.h>
#include <trex/TREXBase.h>
#include <trex/geometry/Config.h>
#include <trex/geometry/LETGeometry.h>
#include <trex/geometry/TREXGeometry.h>

namespace Trex {

class TREXInstrument {
public:
  /// \brief 'create' the TREX instrument
  /// based on settings the constructor loads both configuration
  /// and calibration data. It then initialises event builders and
  /// histograms
  TREXInstrument(Counters &counters, BaseSettings &settings,
                 EV44Serializer &serializer,
                 ESSReadout::Parser &essHeaderParser);

  /// \brief handle loading and application of configuration and calibration
  /// files. This step will throw an exception upon errors.
  void loadConfigAndCalib();

  /// \brief process parsed vmm data into clusters
  void processReadouts(void);

  /// \brief process clusters into events
  void generateEvents(std::vector<Event> &Events);

public:
  /// ADC value histograms for all channels
  Hists ADCHist{1, 1}; // reinit in ctor

  /// \brief One builder per cassette, resize in constructor when we have
  /// parsed the configuration file and know the number of cassettes
  std::vector<EventBuilder2D> builders; // reinit in ctor

  /// \brief parser for VMM3 readout data
  vmm3::VMM3Parser VMMParser;

private:
  Geometry *GeometryInstance;

  /// \brief digital geometry
  /// Defines which digital geometry to use
  /// for calculating pixel ids
  TREXGeometry TREXGeometryInstance;
  LETGeometry LETGeometryInstance;

  /// \brief logical geometry
  /// get pixel IDs from x- and y- coordinates
  ESSGeometry essgeom;

  /// \brief Instrument configuration (rings, FENs, Hybrids, etc)
  Config Conf;

  /// \brief Stuff that 'ties' TREX together
  struct Counters &counters;
  BaseSettings &Settings;

  /// \brief serialiser (and producer) for events
  EV44Serializer &Serializer;

  /// \brief parser for the ESS readout header
  ESSReadout::Parser &ESSHeaderParser;
};

} // namespace Trex
