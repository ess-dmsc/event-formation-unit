// Copyright (C) 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of Pixel observer
//===----------------------------------------------------------------------===//

#include "readout/TimingEventHandler.h"
#include <common/debug/Trace.h>
#include <cstdint>
#include <readout/PixelEventHandler.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

using namespace std;

void PixelEventHandler::applyData(const EpochESSPulseTime &epochEssPulseTime) {
  lastEpochESSPulseTime = make_unique<EpochESSPulseTime>(epochEssPulseTime);
  serializer.setReferenceTime(lastEpochESSPulseTime->pulseTimeInEpochNs);
}

void PixelEventHandler::applyData(const PixelDataEvent &pixelDataEvent) {

  bool ValidData = geometry->validateData(pixelDataEvent);
  if (not ValidData) {
    XTRACE(DATA, WAR, "Invalid Data, skipping readout");
    return;
  }

  if (lastEpochESSPulseTime == nullptr) {
    XTRACE(DATA, WAR, "No epoch pulse time, skipping readout");
    statCounters.NoGlobalTime++;
    return;
  }

  // Calculate TOF in ns
  uint16_t X = geometry->calcX(pixelDataEvent);
  uint16_t Y = geometry->calcY(pixelDataEvent);

  XTRACE(DATA, DEB, "Parsed new hit, ToF: %u, X: %u, Y: %u, ToT: %u",
         pixelDataEvent.fToA, X, Y, pixelDataEvent.ToT);

  uint64_t pixelGlobalTimeStamp = calculateGlobalTime(
      pixelDataEvent.toa, pixelDataEvent.fToA, pixelDataEvent.spidrTime);

  allHitsVector.push_back({pixelGlobalTimeStamp, X, Y, pixelDataEvent.ToT});
}

void PixelEventHandler::pushDataToKafka() {
  // sort hits by time of flight for clustering in time
  sort_chronologically(std::move(allHitsVector));
  clusterer.cluster(allHitsVector);

  ///\todo Decide if flushing per packet is wanted behaviour, or should be
  /// configurable
  clusterer.flush();
  generateEvents();
  allHitsVector.clear();
}

void PixelEventHandler::generateEvents() {
  for (auto cluster : clusterer.clusters) {

    // other options for time are timeEnd, timeCenter, etc. we picked timeStart
    // for this type of
    // detector, it is the time the first photon in the cluster hit the
    // detector.
    uint64_t eventTime = cluster.timeStart();
    uint64_t eventTof = eventTime - lastEpochESSPulseTime->pulseTimeInEpochNs;

    if (eventTof > TimingEventHandler::DEFAULT_FREQUENCY_NS) {
      XTRACE(EVENT, WAR,
             "Event is for the next pulse, EventTime: %u, Current "
             "pulse time: %u, Difference: %u",
             eventTime, lastEpochESSPulseTime->pulseTimeInEpochNs, eventTof);
      statCounters.EventTimeForNextPulse++;
      // continue;
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
  clusterer.clusters.clear();
}

uint64_t PixelEventHandler::calculateGlobalTime(const uint16_t &toa,
                                                const uint8_t &fToA,
                                                const uint32_t &spidrTime) {

  uint64_t pixelClockTime =
      uint64_t(409600 * spidrTime + 25 * toa - 1.5625 * fToA);

  if (lastEpochESSPulseTime->pairedTDCDataEvent.tdcTimeInPixelClock >
      pixelClockTime) {

    uint64_t timeUntilReset =
        PIXEL_MAX_TIMESTAMP_NS -
        lastEpochESSPulseTime->pairedTDCDataEvent.tdcTimeInPixelClock;

    return lastEpochESSPulseTime->pulseTimeInEpochNs + timeUntilReset +
           pixelClockTime;
  } else {
    return lastEpochESSPulseTime->pulseTimeInEpochNs + pixelClockTime -
           lastEpochESSPulseTime->pairedTDCDataEvent.tdcTimeInPixelClock;
  }
}

} // namespace Timepix3
