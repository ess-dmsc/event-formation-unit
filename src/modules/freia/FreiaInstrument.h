// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief FreiaInstrument is responsible for readout validation and event
/// formation
/// Its functions are called from the main processing loop in FreiaBase
//===----------------------------------------------------------------------===//

#pragma once

#include <common/readout/ess/Parser.h>
#include <common/readout/vmm3/Readout.h>
#include <common/readout/vmm3/VMM3Parser.h>
#include <common/reduction/EventBuilder2D.h>
#include <freia/geometry/Config.h>
#include <freia/geometry/Geometry.h>

#include <memory>
#include <vector>

// Forward declarations
class EV44Serializer;
class Event;
struct BaseSettings;
struct Counters;

namespace Freia {

class FreiaInstrument {

private:
  /// \brief Stuff that 'ties' Freia together
  Counters &counters;
  BaseSettings &Settings;

  /// \brief Instrument configuration (rings, cassettes, FENs)
  Config Conf;

  /// \brief digital geometry
  /// get x- and y- coordinates from cassettes and channels
  Geometry Geom;

  /// \brief serialiser (and producer) for events
  EV44Serializer &Serializer;

  /// \brief parser for the ESS Readout header
  ESSReadout::Parser &ESSHeaderParser;

public:
  /// \brief 'create' the Freia instrument
  /// based on settings the constructor loads both configuration
  /// and calibration data. It then initialises event builders and
  /// histograms
  FreiaInstrument(Counters &counters, BaseSettings &settings,
                  EV44Serializer &serializer,
                  ESSReadout::Parser &essHeaderParser);

  /// \brief handle loading and application of configuration and calibration
  /// files. This step will throw an exception upon errors.
  void loadConfigAndCalib();

  /// \brief process parsed vmm data into clusters
  void processReadouts(void);

  /// \brief process clusters into events
  void generateEvents(std::vector<Event> &Events);

  /// \brief One builder per cassette, resize in constructor when we have
  /// parsed the configuration file and know the number of cassettes
  std::vector<EventBuilder2D> builders; // reinit in ctor

  /// \brief parser for VMM3 readout data
  ESSReadout::VMM3Parser VMMParser;
};

} // namespace Freia
