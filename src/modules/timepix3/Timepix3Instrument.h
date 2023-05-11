// Copyright (C) 2020-2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating Timepix3 processing from pipeline main loop
///
/// Holds efu stats, instrument readout mappings, logical geometry, pixel
/// calculations and Timepix3 readout parser
//===----------------------------------------------------------------------===//

#pragma once

#include <common/readout/ess/ESSTime.h>
#include <common/readout/ess/Parser.h>
#include <readout/DataParser.h>
#include <timepix3/Counters.h>
#include <timepix3/Timepix3Base.h> // to get Timepix3Settings
#include <timepix3/geometry/Config.h>
#include <timepix3/geometry/Geometry.h>

namespace Timepix3 {

struct TimepixHit {
  uint32_t X;
  uint32_t Y;
  uint64_t TimeOfFlight;
  uint16_t ToT;
};

class Timepix3Instrument {
public:
  /// \brief 'create' the Timepix3 instruments
  ///
  /// loads configuration and calibration files, calulate and generate the
  /// logical geometry and initialise the amplitude to position calculations
  Timepix3Instrument(Counters &counters, BaseSettings &settings);

  ~Timepix3Instrument();

  /// \brief Generates Hits from Readouts, and adds them to event builder
  void processReadouts();

  /// \brief Sets the serializer to send events to
  void setSerializer(EV44Serializer *serializer) { Serializer = serializer; }

  /// \brief Timepix3 pixel calculations
  uint32_t calcPixel(DataParser::Timepix3PixelReadout &Data);

  /// \brief Timepix3 tof calculations
  uint64_t calcTimeOfFlight(DataParser::Timepix3PixelReadout &Data);

  /// \brief writes a single readout to file
  void dumpReadoutToFile(DataParser::Timepix3PixelReadout &Data);

  void formEvents(std::vector<TimepixHit> ClusteredInTimeEventHits);

public:
  /// \brief Stuff that 'ties' Timepix3 together
  struct Counters &counters;

  Config Timepix3Configuration;
  BaseSettings &Settings;
  DataParser Timepix3Parser{counters};
  Geometry *Geom;
  EV44Serializer *Serializer;
  // TODO, fix this std::shared_ptr<ReadoutFile> DumpFile;
};

} // namespace Timepix3
