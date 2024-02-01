// Copyright (C) 2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation for Timepix3 pixel global time calculation and
///        clustering.
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <common/reduction/Hit2DVector.h>
#include <common/reduction/clustering/Abstract2DClusterer.h>
#include <common/reduction/clustering/Hierarchical2DClusterer.h>
#include <handlers/PixelEventHandler.h>
#include <handlers/TimingEventHandler.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

using namespace std;
using namespace timepixReadout;
using namespace timepixDTO;
using namespace efutils;

PixelEventHandler::PixelEventHandler(Counters &statCounters,
                                     shared_ptr<Timepix3Geometry> geometry,
                                     EV44Serializer &serializer)
    : statCounters(statCounters), geometry(geometry), serializer(serializer) {

  clusterers.resize(geometry->getChunkNumber());
  windows.resize(geometry->getChunkNumber());

  for (int i = 0; i < geometry->getChunkNumber(); i++) {
    clusterers[i] = std::make_unique<Hierarchical2DClusterer>(
        Hierarchical2DClusterer(1, 5));
    windows[i] = Hit2DVector();
  }
}

void PixelEventHandler::applyData(const ESSGlobalTimeStamp &epochEssPulseTime) {

  lastEpochESSPulseTime = make_unique<ESSGlobalTimeStamp>(epochEssPulseTime);
  serializer.setReferenceTime(lastEpochESSPulseTime->pulseTimeInEpochNs);
}

void PixelEventHandler::applyData(const PixelReadout &pixelReadout) {

  bool ValidData = geometry->validateData(pixelReadout);
  if (not ValidData) {
    XTRACE(DATA, WAR, "Invalid Data, skipping readout");
    statCounters.InvalidPixelReadout++;
    return;
  }

  if (lastEpochESSPulseTime == nullptr) {
    XTRACE(DATA, WAR, "No epoch pulse time, skipping readout");
    statCounters.NoGlobalTime++;
    return;
  }

  // Calculate TOF in ns
  uint16_t X = geometry->calcX(pixelReadout);
  uint16_t Y = geometry->calcY(pixelReadout);

  XTRACE(DATA, DEB, "Parsed new hit, ToF: %u, X: %u, Y: %u, ToT: %u",
         pixelReadout.fToA, X, Y, pixelReadout.ToT);

  uint64_t pixelGlobalTimeStamp = calculateGlobalTime(
      pixelReadout.toa, pixelReadout.fToA, pixelReadout.spidrTime);

  int windowIndex = geometry->getChunkWindowIndex(X, Y);
  // Add the hit to the corresponding window vector

  windows[windowIndex].push_back(
      {pixelGlobalTimeStamp, X, Y, pixelReadout.ToT});
}

void PixelEventHandler::clusterHits(Hierarchical2DClusterer &clusterer,
                                    Hit2DVector &hitsVector) {

  // sort hits by time of flight for clustering in time
  sort_chronologically(std::move(hitsVector));
  clusterer.cluster(hitsVector);
  clusterer.flush();
}

void PixelEventHandler::pushDataToKafka() {

  std::vector<std::future<void>> futures;

  int windowsLength = windows.size();

  if (windowsLength == 1) {
    clusterHits(*clusterers[0], windows[0]);
  } else {
    for (int i = 0; i < windowsLength; i++) {
      auto &window = windows[i];
      if (window.size() > 0) {
        futures.push_back(
            std::async(std::launch::async, &PixelEventHandler::clusterHits,
                       this, std::ref(*clusterers[i]), std::ref(window)));
      }
    }

    if (!futures.empty()) {
      for (auto &future : futures) {
        future.wait();
      }
    }
  }

  for (auto &window : windows) {
    window.clear();
  }

  for (auto &cluster : clusterers) {
    publishEvents(cluster->clusters);
  }
}

void PixelEventHandler::publishEvents(Cluster2DContainer &clusters) {
  for (auto cluster : clusters) {

    // other options for time are timeEnd, timeCenter, etc. we picked timeStart
    // for this type of
    // detector, it is the time the first photon in the cluster hit the
    // detector.
    uint64_t  eventTime = cluster.timeStart();
    long eventTof = eventTime - lastEpochESSPulseTime->pulseTimeInEpochNs;
    statCounters.TofCount++;

    if (eventTof < 0) {
      statCounters.TofNegative++;
      continue;
    }

    if (eventTof > TimingEventHandler::DEFAULT_FREQUENCY_NS) {
      XTRACE(EVENT, WAR,
             "Event is for the next pulse, EventTime: %u, Current "
             "pulse time: %u, Difference: %u",
             eventTime, lastEpochESSPulseTime->pulseTimeInEpochNs, eventTof);
      statCounters.EventTimeForNextPulse++;
      continue;
    }
    uint16_t x = cluster.xCoordCenter();
    uint16_t y = cluster.yCoordCenter();
    uint32_t PixelId = geometry->pixel2D(x, y);

    if (PixelId == 0) {
      XTRACE(EVENT, WAR, "Bad pixel!: Time: %u, x %u, y %u, pixel %u",
             eventTime, x, y, PixelId);
      statCounters.PixelErrors++;
      continue;
    }
    XTRACE(EVENT, DEB, "New event, Time: %u, PixelId: %u", eventTime, PixelId);
    statCounters.TxBytes += serializer.addEvent(eventTof, PixelId);
    statCounters.Events++;
  }
  clusters.clear();
}

uint64_t PixelEventHandler::calculateGlobalTime(const uint16_t &toa,
                                                const uint8_t &fToA,
                                                const uint32_t &spidrTime) {

  uint64_t pixelClockTime =
      uint64_t(409600 * spidrTime + 25 * toa - 1.5625 * fToA);

  if (lastEpochESSPulseTime->tdcClockInPixelTime > pixelClockTime) {

    uint64_t timeUntilReset =
        PIXEL_MAX_TIMESTAMP_NS - lastEpochESSPulseTime->tdcClockInPixelTime;

    return lastEpochESSPulseTime->pulseTimeInEpochNs + timeUntilReset +
           pixelClockTime;
  } else {
    return lastEpochESSPulseTime->pulseTimeInEpochNs + pixelClockTime -
           lastEpochESSPulseTime->tdcClockInPixelTime;
  }
}
} // namespace Timepix3
