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

#include "Counters.h"
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

  if (newTdcData->counter != lastTDCData->counter + 1) {
    statCounters.MissTDCCounter += newTdcData->counter - lastTDCData->counter;
  }

  lastTDCData = newTdcData;

  if (isLastTimingDiffLowerThenThreshold()) {

    globalTime = shared_ptr<GlobalTime>(
        new GlobalTime(lastEVRData->pulseTimeSeconds,
                       lastEVRData->pulseTimeNanoSeconds, *lastTDCData));
  }
}

void TimingEventHandler::applyData(const shared_ptr<EVRDataEvent> &newEVRData) {

  XTRACE(EVENT, DEB, "New EVR Data with PulseTime, s: %u, ns: %u, arrival: %u",
         lastEVRData->pulseTimeSeconds, lastEVRData->pulseTimeNanoSeconds,
         lastEVRData->arrivalTimestamp.time_since_epoch());

  if (newEVRData->counter != lastEVRData->counter + 1) {
    statCounters.MissEVRCounter += newEVRData->counter - lastEVRData->counter;
  }

  lastEVRData = newEVRData;

  if (isLastTimingDiffLowerThenThreshold()) {
    globalTime = shared_ptr<GlobalTime>(
        new GlobalTime(lastEVRData->pulseTimeSeconds,
                       lastEVRData->pulseTimeNanoSeconds, *lastTDCData));
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

} // namespace Timepix3