// Copyright (C) 2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation for Timepix3 pixel global time calculation and
///        clustering.
//===----------------------------------------------------------------------===//

#include "common/reduction/Hit2DVector.h"
#include <common/debug/Trace.h>
#include <memory>
#include <timepix3/handlers/PixelEventHandler.h>
#include <timepix3/handlers/TimingEventHandler.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

using namespace std;
using namespace timepixReadout;
using namespace timepixDTO;
using namespace efutils;

PixelEventHandler::PixelEventHandler(Counters &statCounters,
                                     shared_ptr<Timepix3Geometry> geometry,
                                     EV44Serializer &serializer,
                                     const Config &timepix3Configuration)
    : statCounters(statCounters), geometry(geometry), serializer(serializer),
      TimepixConfiguration(timepix3Configuration),
      FrequencyPeriodNs(hzToNanoseconds(timepix3Configuration.FrequencyHz)),
      Hits(std::make_unique<Hit2DVector>()) { // Initialize Hits with unique_ptr

  clusterer = std::make_unique<Hierarchical2DClusterer>(
      Hierarchical2DClusterer(TimepixConfiguration.MaxTimeGapNS,
                              timepix3Configuration.MaxCoordinateGap));
}

void PixelEventHandler::applyData(const ESSGlobalTimeStamp &epochEssPulseTime) {
  serializer.setReferenceTime(epochEssPulseTime.pulseTimeInEpochNs);
}

void PixelEventHandler::clusterHits(Hierarchical2DClusterer &clusterer,
                                    Hit2DVector &hitsVector) {

  // sort hits by time of flight for clustering in time
  sort_chronologically(std::move(hitsVector));
  clusterer.cluster(hitsVector);
  clusterer.flush();
}

void PixelEventHandler::pushDataToKafka() {
  clusterHits(*clusterer, *Hits); // Dereference shared_ptr to access the vector
  publishEvents(clusterer->clusters);
}

void PixelEventHandler::publishEvents(Cluster2DContainer &clusters) {
  for (auto cluster : clusters) {

    // other options for time are timeEnd, timeCenter, etc. we picked timeStart
    // for this type of
    // detector, it is the time the first photon in the cluster hit the
    // detector.

    if (cluster.hitCount() < TimepixConfiguration.MinEventSizeHits ||
        cluster.weightSum() < TimepixConfiguration.MinimumToTSum) {

      statCounters.ClusterSizeTooSmall++;
      continue;
    }

    if (cluster.timeSpan() < TimepixConfiguration.MinEventTimeSpan) {
      statCounters.ClusterToShort++;
      continue;
    }

    uint64_t eventTime = cluster.timeStart();
    long eventTofNs = static_cast<long>(eventTime);
    statCounters.TofCount++;

    if (eventTofNs < 0) {
      statCounters.TofNegative++;
      continue;
    }

    // If the event would be the next pulse, move it to the beginning of the
    // current pulse to keep up statistics
    if (eventTofNs > FrequencyPeriodNs.count()) {
      statCounters.EventTimeForNextPulse++;
      // Normalize the event time to the current pulse
      eventTofNs =
          eventTofNs %
          FrequencyPeriodNs.count(); // Ensure eventTofNs is within the period
    }

    double EventCoordX = cluster.xCoordCenter();
    double EventCoordY = cluster.yCoordCenter();
    uint32_t PixelId = geometry->calcPixelId(EventCoordX, EventCoordY);

    if (PixelId == 0) {
      XTRACE(EVENT, WAR, "Bad pixel!: Time: %u, x %f, y %f, pixelid %u",
             eventTime, EventCoordX, EventCoordY, PixelId);
      statCounters.PixelErrors++;
      continue;
    }
    XTRACE(EVENT, DEB, "New event, Time: %u, PixelId: %u", eventTime, PixelId);
    serializer.addEvent(eventTofNs, PixelId);
    statCounters.Events++;
  }
  clusters.clear();
}
} // namespace Timepix3
