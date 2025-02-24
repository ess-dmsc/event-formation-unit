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

#include <common/kafka/EV44Serializer.h>
#include <common/readout/ess/Parser.h>
#include <common/readout/vmm3/Hybrid.h>
#include <common/readout/vmm3/Readout.h>
#include <common/readout/vmm3/VMM3Parser.h>
#include <common/reduction/EventBuilder2D.h>
#include <freia/Counters.h>
#include <freia/FreiaBase.h>
#include <freia/geometry/Config.h>
#include <freia/geometry/Geometry.h>

namespace Freia {

class FreiaInstrument {
public:
  /// \brief 'create' the Freia instrument
  /// based on settings the constructor loads both configuration
  /// and calibration data. It then initialises event builders and
  /// histograms
  FreiaInstrument(Counters &counters, BaseSettings &settings,
                  EV44Serializer *serializer);

  /// \brief handle loading and application of configuration and calibration
  /// files. This step will throw an exception upon errors.
  void loadConfigAndCalib();

  /// \brief after loading the config file, Config.HybridIdStr contains
  /// a vector of HybridIds. These are then loaded into the Hybrids so that
  /// we can later do consistency checks when applying the calibration data
  void setHybridIds(const std::vector<std::string> &Ids);

  /// \brief process parsed vmm data into clusters
  void processReadouts(void);

  /// \brief process clusters into events
  void generateEvents(std::vector<Event> &Events);

  /// \brief dump readout data to HDF5
  void dumpReadoutToFile(const ESSReadout::VMM3Parser::VMM3Data &Data);

  /// \brief initialise the serializer. This is used both in FreiaInstrument
  // and FreiaBase. Called from FreiaBase
  void setSerializer(EV44Serializer *serializer) { Serializer = serializer; }

public:
  /// \brief Stuff that 'ties' Freia together
  struct Counters &counters;
  BaseSettings &Settings;

  /// \brief serialiser (and producer) for events
  EV44Serializer *Serializer{nullptr};

  /// \brief One builder per cassette, resize in constructor when we have
  /// parsed the configuration file and know the number of cassettes
  std::vector<EventBuilder2D> builders; // reinit in ctor

  /// \brief Instrument configuration (rings, cassettes, FENs)
  Config Conf;

  /// \brief digital geometry
  /// get x- and y- coordinates from cassettes and channels
  Geometry Geom;

  // Each cassette holds 2 VMMCalibrations
  std::vector<ESSReadout::Hybrid> Hybrids;

  /// \brief parser for the ESS Readout header
  ESSReadout::Parser ESSReadoutParser;

  /// \brief parser for VMM3 readout data
  ESSReadout::VMM3Parser VMMParser;

  /// \brief for dumping raw VMM3 readouts to HDF5 files
  std::shared_ptr<VMM3::ReadoutFile> DumpFile;
};

} // namespace Freia
