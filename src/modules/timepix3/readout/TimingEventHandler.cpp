// Copyright (C) 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation for TDC and EVR timing event observers. Responsible
//         to follow up different timing events and syncronize the two timing
//         (ESS time and camera time) domains. Also this class provides timing
//         information for other timing interested objects.
//===----------------------------------------------------------------------===//

#include "readout/DataEventTypes.h"
#include <common/debug/Trace.h>
#include <cstdint>
#include <memory>
#include <readout/TimingEventHandler.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

using namespace chrono;
using namespace efutils;

const uint32_t TimingEventHandler::DEFAULT_FREQUENCY_NS =
    hzToNanoseconds(14).count();

void TimingEventHandler::applyData(const shared_ptr<TDCDataEvent> &newTdcData) {

  XTRACE(EVENT, DEB,
         "New TDC Data with Timestamp: %u, Frequency: %u, arrival: %u",
         lastTDCData->tdcTimeStamp, tdcRepetitionFrequency,
         lastTDCData->arrivalTimestamp.time_since_epoch());

    if (isLastTimingDiffLowerThenThreshold()) {
      lastTDCPair = lastTDCData;
      statCounters.FoundEVRandTDCPairs++;

      XTRACE(EVENT, DEB,
             "I found EVR pair with previous EVR. TDC counter: %u, EVR "
             "counter: %u ArrivalDiff: %u",
             lastTDCData->counter, lastEVRData->Counter,
             abs(duration_cast<milliseconds>(lastTDCData->arrivalTimestamp -
                                             lastEVRData->arrivalTimestamp)

                     .count()));
      lookingForNextTDC = false;
    } else {
      XTRACE(EVENT, DEB, "No TDC pair found for EVR: %u", lastEVRData->Counter);
    }
  } else {
    if (lastTDCData != NULL && lastTDCPair != lastTDCData) {
      XTRACE(EVENT, DEB, "Miss EVR pair for TDC arrived: %u, with counter: %u",
             lastTDCData->arrivalTimestamp.time_since_epoch(),
             lastTDCData->counter);

      statCounters.MissEVRPair++;
    }

    lastTDCData = std::make_unique<TDCDataEvent>(newTdcData);
  }
}

void TimingEventHandler::applyData(const EVRDataEvent &newData) {

  lastEVRData = std::make_unique<EVRDataEvent>(newData);

  if (lookingForNextTDC == true) {
    XTRACE(EVENT, DEB, "Miss TDC pair for EVR arrived: %u, with counter: %u",
           lastEVRData->arrivalTimestamp.time_since_epoch(),
           lastEVRData->Counter);

    statCounters.MissTDCPair++;
  }

  lookingForNextTDC = true;

  XTRACE(EVENT, DEB, "New EVR Data with PulseTime, s: %u, ns: %u, arrival: %u",
         lastEVRData->PulseTimeSeconds, lastEVRData->PulseTimeNanoSeconds,
         lastEVRData->arrivalTimestamp.time_since_epoch());

  if (lastTDCData != nullptr) {

    if (isLastTimingDiffLowerThenThreshold()) {

      lastTDCPair = lastTDCData;
      lookingForNextTDC = false;
      statCounters.FoundEVRandTDCPairs++;

      XTRACE(EVENT, DEB,
             "I found TDC pair with previous TDC. TDC counter: %u, EVR "
             "counter: %u ArrivalDiff: %u",
             lastTDCData->counter, lastEVRData->Counter,
             abs(duration_cast<milliseconds>(lastTDCData->arrivalTimestamp -
                                             lastEVRData->arrivalTimestamp)
                     .count()));
    }
  }
}

uint64_t TimingEventHandler::getLastTDCTimestamp() const {
  if (lastTDCData != NULL) {
    return lastTDCData->tdcTimeStamp;
  } else {
    return 0;
  }
}

uint32_t TimingEventHandler::getTDCFrequency() const {
  return tdcRepetitionFrequency;
}

const std::shared_ptr<TDCDataEvent>
TimingEventHandler::getLastTdcEvent() const {
  return lastTDCData;
}

const std::shared_ptr<EVRDataEvent>
TimingEventHandler::getLastEvrEvent() const {
  return lastEVRData;
}

const shared_ptr<TDCDataEvent> TimingEventHandler::getLastTDCPair() const {
  return lastTDCPair;
}

} // namespace Timepix3