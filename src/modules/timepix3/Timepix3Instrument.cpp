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

#include "readout/TimepixDataTypes.h"
#include "readout/DataParser.h"
#include <common/debug/Trace.h>
#include <timepix3/Timepix3Instrument.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

using namespace Observer;
using namespace timepixDTO;
using namespace timepixReadout;

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
                                       Config timepix3Configuration,
                                       EV44Serializer &serializer)
    : counters(counters), serializer(serializer),
      clusterer(timepix3Configuration.MaxTimeGapNS,
                timepix3Configuration.MaxCoordinateGap),
      geomPtr(std::make_shared<Timepix3Geometry>(
          timepix3Configuration.XResolution, timepix3Configuration.YResolution,
          timepix3Configuration.NumberOfChunks)),
      timingEventHandler(counters),
      pixelEventHandler(counters, geomPtr, serializer),
      timepix3Parser(counters) {

  // Setup observable subscriptions
  timepix3Parser.DataEventObservable<TDCReadout>::subscribe(&timingEventHandler);
  timepix3Parser.DataEventObservable<EVRReadout>::subscribe(&timingEventHandler);

  timepix3Parser.DataEventObservable<PixelReadout>::subscribe(
      &pixelEventHandler);
  timingEventHandler.DataEventObservable<ESSGlobalTimeStamp>::subscribe(
      &pixelEventHandler);
}

void Timepix3Instrument::processReadouts() {
  XTRACE(DATA, DEB, "Processing readouts");
  pixelEventHandler.pushDataToKafka();
}

Timepix3Instrument::~Timepix3Instrument() {}

} // namespace Timepix3