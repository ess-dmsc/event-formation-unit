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

#include "common/reduction/Hit2DVector.h"
#include "efu/DataPipeline.h"
#include <common/detector/BaseSettings.h>
#include <common/kafka/EV44Serializer.h>
#include <common/reduction/clustering/Hierarchical2DClusterer.h>
#include <dataflow/DataObserverTemplate.h>
#include <geometry/Config.h>
#include <handlers/PixelEventHandler.h>
#include <handlers/TimingEventHandler.h>
#include <memory>
#include <modules/timepix3/Counters.h>
#include <readout/DataParser.h>

namespace Timepix3 {

class Timepix3Instrument {
public:
  /// \brief 'create' the Timepix3 instruments
  ///
  /// loads configuration and calibration files, calulate and generate the
  /// logical geometry and initialise the amplitude to position calculations

  /// \brief calculate pixel ID from a Timepix3PixelReadout
  uint32_t calcPixel(timepixReadout::PixelReadout &Data);

  Observer::DataEventObservable<timepixDTO::ESSGlobalTimeStamp>
      epochESSPulseTimeObservable;

  /// \brief from the clusters in Clusterer, check if they meet requirements to
  /// be an event, and if so serialize them
  struct Counters &counters;
  EV44Serializer &serializer;
  Hierarchical2DClusterer clusterer;
  shared_ptr<Timepix3Geometry> geomPtr;
  TimingEventHandler timingEventHandler;
  PixelEventHandler pixelEventHandler;
  DataParser timepix3Parser;

  int MaxTimeGapNS;
  int MaxCoordinateGap;

  data_pipeline::Pipeline DataPipeline;

  Timepix3Instrument(Counters &counters, const Config &timepix3Configuration,
                     EV44Serializer &serializer);

  ~Timepix3Instrument();
  // Load configuration and calibration files, calculate and generate the
  // logical geometry, and initialize the amplitude to position calculations
};

} // namespace Timepix3
