// Copyright (C) 2024 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief NMXInstrument is responsible for readout validation and event
/// formation
/// Its functions are called from the main processing loop in NMXBase
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
#include <nmx/Counters.h>
#include <nmx/NMXBase.h>
#include <nmx/geometry/Config.h>
#include <nmx/geometry/NMXGeometry.h>

namespace Nmx {

class NMXInstrument {
public:
  /// \brief 'create' the NMX instrument
  /// based on settings the constructor loads both configuration
  /// and calibration data. It then initialises event builders and
  /// histograms
  NMXInstrument(Counters &counters, BaseSettings &Settings,
                EV44Serializer *serializer);

  /// \brief handle loading and application of configuration and calibration
  /// files. This step will throw an exception upon errors.
  void loadConfigAndCalib();

  /// \brief process parsed vmm data into clusters
  void processReadouts(void);

  /// \brief process clusters into events
  void generateEvents(std::vector<Event> &Events);

  /// \brief initialise the serializer. This is used both in NMXInstrument
  // and NMXBase. Called from NMXBase
  void setSerializer(EV44Serializer *serializer) { Serializer = serializer; }

  ///\brief ensures the combination of config and geometry doesn't result
  /// in overlapping pixels. If it does, throws a runtime error.
  void checkConfigAndGeometry();

public:
  /// \brief Stuff that 'ties' NMX together
  struct Counters &counters;

  BaseSettings &Settings;

  /// \brief serialiser (and producer) for events
  EV44Serializer *Serializer{nullptr};

  /// ADC value histograms for all channels
  Hists ADCHist{1, 1}; // reinit in ctor

  /// \brief One builder per cassette, resize in constructor when we have
  /// parsed the configuration file and know the number of cassettes
  std::vector<EventBuilder2D> builders; // reinit in ctor

  /// \brief Instrument configuration (rings, FENs, Hybrids, etc)
  Config Conf;

  /// \brief logical geometry
  /// get pixel IDs from x- and y- coordinates
  ESSGeometry essgeom;

  /// \brief digital geometry
  /// Defines which digital geometry to use
  /// for calculating pixel ids
  NMXGeometry NMXGeometryInstance;

  Geometry *GeometryInstance;

  //
  std::vector<ESSReadout::Hybrid> Hybrids;

  /// \brief parser for the ESS Readout header
  ESSReadout::Parser ESSReadoutParser;

  /// \brief parser for VMM3 readout data
  ESSReadout::VMM3Parser VMMParser;
};

} // namespace Nmx
