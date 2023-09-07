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
#include <common/reduction/Hit2DVector.h>
#include <common/reduction/clustering/Hierarchical2DClusterer.h>
#include <readout/DataParser.h>
#include <timepix3/Counters.h>
#include <timepix3/Timepix3Base.h> // to get Timepix3Settings
#include <timepix3/geometry/Config.h>
#include <timepix3/geometry/Timepix3Geometry.h>

namespace Timepix3 {

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

  /// \brief calculate pixel ID from a Timepix3PixelReadout
  uint32_t calcPixel(DataParser::Timepix3PixelReadout &Data);

  /// \brief calculate the ToF (difference between pulse time and time of
  /// arrival) from a single Timepix3PixelReadout
  uint64_t calcTimeOfFlight(DataParser::Timepix3PixelReadout &Data);

  /// \brief from the clusters in Clusterer, check if they meet requirements to
  /// be an event, and if so serialize them
  void generateEvents();

public:
  /// \brief Stuff that 'ties' Timepix3 together
  struct Counters &counters;

  Config Timepix3Configuration;
  BaseSettings &Settings;
  DataParser Timepix3Parser{counters};
  Timepix3Geometry *Geom;
  EV44Serializer *Serializer;
  Hierarchical2DClusterer *Clusterer;
  Hit2DVector AllHitsVector;
};

} // namespace Timepix3
