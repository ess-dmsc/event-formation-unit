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

#include <chrono>
#include <common/debug/Trace.h>
#include <cstdint>
#include <ctime>
#include <dto/TimepixDataTypes.h>
#include <efu/DataPipeline.h>
#include <efu/ThreadPool.hpp>
#include <future>
#include <queue>
#include <timepix3/Timepix3Instrument.h>
#include <utility>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

using namespace Observer;
using namespace timepixDTO;
using namespace timepixReadout;
using namespace std;
using namespace std::chrono;
using namespace data_pipeline;

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

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
      MaxTimeGapNS(timepix3Configuration.MaxTimeGapNS),
      MaxCoordinateGap(timepix3Configuration.MaxCoordinateGap),
      DataPipeline(
          data_pipeline::PipelineBuilder()
              .addStage<PipelineStage<std::vector<uint64_t>, Hit2DVector>>(
                  [this](std::vector<uint64_t> &data) {
                    return timepix3Parser.parseTPX(data);
                  })
              .addStage<PipelineStage<Hit2DVector, std::future<Hit2DVector>>>(
                  [this](Hit2DVector &hits) {

                    return ThreadPool::getInstance().enqueue(
                        [this, hits = std::move(hits)]() mutable {
                            sort_chronologically(hits);
                            return hits;
                        });
                  })
              .addStage<
                  PipelineStage<std::future<Hit2DVector>, std::future<Cluster2DContainer>>>(
                  [this](std::future<Hit2DVector> &future) {

                    auto hits = std::move(future.get());

                    // Start a new thread to cluster the hits parallel. Ensure
                    // that hits ownership moved over to new thread. After
                    // initiaalizing the clustererm thread this stage returns a
                    // future (promise) of the result to the next stage
                    return ThreadPool::getInstance().enqueue(
                        [this, hits = std::move(hits)]() mutable {
                          Hierarchical2DClusterer ThreadClusterer(
                              MaxTimeGapNS, MaxCoordinateGap);
                          ThreadClusterer.cluster(hits);
                          ThreadClusterer.flush();
                          return ThreadClusterer.getClusters();
                        });
                  })
                .addStage<PipelineStage<std::future<Cluster2DContainer>, Cluster2DContainer>>(
                  [this](std::future<Cluster2DContainer> &future) {
                    // This stage waits for the future to be ready and then
                    // returns the result to the next stage
                    return future.get();
                  })
                .addStage<PipelineStage<Cluster2DContainer, int>>(
                  [this](Cluster2DContainer &clusters) {
                    // Process step by step the future promise stored in the
                    // input queue. Wait for the future to be ready and then
                    // publish the events
                    return pixelEventHandler.publishEvents(clusters);
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