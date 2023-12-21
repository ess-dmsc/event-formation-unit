// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating Timepix3 processing from pipeline main loop
///
/// Holds efu stats, instrument readout mappings, logical geometry, pixel
/// calculations and Timepix3 readout parser
//===----------------------------------------------------------------------===//

#include "dataflow/DataObserverTemplate.h"
#include "readout/DataEventTypes.h"
#include <common/debug/Trace.h>
#include <timepix3/Timepix3Instrument.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

/**
 * @brief Constructs a Timepix3Instrument object.
 *
 * This constructor initializes a Timepix3Instrument object with the provided
 * counters, settings, and serializer. It also sets up observable subscriptions
 * for timing events, pixel events, and epoch ESS pulse time events.
 *
 * @param counters The Counters object used for counting events.
 * @param settings The BaseSettings object containing configuration settings.
 * @param serializer The EV44Serializer object used for serialization.
 */
Timepix3Instrument::Timepix3Instrument(Counters &counters,
                                       BaseSettings &settings,
                                       EV44Serializer &serializer)
    : counters(counters), serializer(serializer),
      timepix3Configuration(Config(settings.ConfigFile)),
      clusterer(timepix3Configuration.MaxTimeGapNS,
                timepix3Configuration.MaxCoordinateGap),
      geomPtr(std::make_shared<Timepix3Geometry>(
          timepix3Configuration.XResolution, timepix3Configuration.YResolution,
          1, 1)),
      timingEventHandler(counters, serializer, epochESSPulseTimeObservable),
      pixelEventHandler(counters, geomPtr, clusterer, serializer),
      timepix3Parser(counters, tdcDataObservable, evrDataObservable,
                     pixelDataObservable) {

  // Setup observable subscriptions
  tdcDataObservable.subscribe(&timingEventHandler);
  evrDataObservable.subscribe(&timingEventHandler);

  pixelDataObservable.subscribe(&pixelEventHandler);
  epochESSPulseTimeObservable.subscribe(&pixelEventHandler);
}

Timepix3Instrument::~Timepix3Instrument() {}

void Timepix3Instrument::processReadouts() {
  XTRACE(DATA, DEB, "Processing readouts");
  pixelEventHandler.pushDataToKafka();
}

// /// \brief helper function to calculate pixels from timepix3 data
// uint32_t Timepix3Instrument::calcPixel(PixelDataEvent &Data) {
//   XTRACE(DATA, DEB, "Calculating pixel");

//   uint32_t pixel = geomPtr->calcPixel(Data);
//   XTRACE(DATA, DEB, "Calculated pixel to be %u", pixel);
//   return pixel;
// }
} // namespace Timepix3