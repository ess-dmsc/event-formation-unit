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

#include "common/reduction/Hit2DVector.h"
#include <common/debug/Trace.h>
#include <cstdint>
#include <ctime>
#include <dto/TimepixDataTypes.h>
#include <timepix3/Timepix3Instrument.h>
#include <utility>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

using namespace Observer;
using namespace timepixDTO;
using namespace timepixReadout;

///
/// \brief Constructs a Timepix3Instrument object.
///
/// This constructor initializes a Timepix3Instrument object with the provided
/// counters, settings, and serializer. It also sets up observable subscriptions
/// for timing events, pixel events, and epoch ESS pulse time events.
///
/// \param counters The Counters object used for counting events.
/// \param settings The BaseSettings object containing configuration settings.
/// \param serializer The EV44Serializer object used for serialization.
///
Timepix3Instrument::Timepix3Instrument(Counters &counters,
                                       const Config &timepix3Configuration,
                                       EV44Serializer &serializer)
    : counters(counters), serializer(serializer),
      clusterer(timepix3Configuration.MaxTimeGapNS,
                timepix3Configuration.MaxCoordinateGap),
      geomPtr(std::make_shared<Timepix3Geometry>(
          timepix3Configuration.XResolution, timepix3Configuration.YResolution,
          timepix3Configuration.ScaleUpFactor,
          timepix3Configuration.parallelThreads)),
      timingEventHandler(counters, timepix3Configuration.FrequencyHz),
      pixelEventHandler(counters, geomPtr, serializer, timepix3Configuration),
      timepix3Parser(counters, geomPtr),
      DataPipeline(
          data_pipeline::PipelineBuilder()
              .addStage<data_pipeline::PipelineStage<std::vector<uint64_t>,
                                                     std::vector<Hit2D>>>(
                  [this](std::vector<uint64_t> &&data) {
                    return timepix3Parser.parseTPX(data);
                  })
              .addStage<data_pipeline::PipelineStage<std::vector<Hit2D>, bool>>(
                  [this](std::vector<Hit2D> &&hits) {
                    return pixelEventHandler.pushDataToKafka(hits);
                  })
              .build()) {

  // Setup observable subscriptions
  timepix3Parser.DataEventObservable<TDCReadout>::subscribe(
      &timingEventHandler);
  timepix3Parser.DataEventObservable<EVRReadout>::subscribe(
      &timingEventHandler);

  timingEventHandler.DataEventObservable<ESSGlobalTimeStamp>::subscribe(
      &pixelEventHandler);
  timingEventHandler.DataEventObservable<ESSGlobalTimeStamp>::subscribe(
      &timepix3Parser);

  DataPipeline.start();
}

Timepix3Instrument::~Timepix3Instrument() {}

} // namespace Timepix3