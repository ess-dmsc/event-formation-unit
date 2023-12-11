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

void TimingEventHandler::applyData(const TDCDataEvent &newTdcData) {

  /// \todo review this is required or beter to go with fixed frequency from
  /// config file. Test required on camera data
  updateTDCFrequency(newTdcData);

  // Calculate the next TDC according to requency or if the calculated value
  // higher then max TDC_MAX_TIMESTAMP_NS clock, then calculate it according to
  // clock reset.
  nextTDCTimeStamp =
      newTdcData.tdcTimeStamp + tdcRepetitionFrequency > TDC_MAX_TIMESTAMP_NS
          ? newTdcData.tdcTimeStamp + tdcRepetitionFrequency -
                TDC_MAX_TIMESTAMP_NS
          : newTdcData.tdcTimeStamp + tdcRepetitionFrequency;

  XTRACE(EVENT, DEB,
         "New TDC Data with Timestamp: %u, Frequency: %u, arrival: %u",
         lastTdcData->tdcTimeStamp, tdcRepetitionFrequency,
         lastTdcData->arrivalTimestamp.time_since_epoch());

  if (lookingForNextTDC) {
    lastTdcData = std::make_unique<TDCDataEvent>(newTdcData);

    if (isLastTimingDiffLowerThenThreshold()) {
      lastTDCPair = lastTdcData;
      statCounters.FoundEVRandTDCPairs++;

      XTRACE(EVENT, DEB,
             "I found EVR pair with previous EVR. TDC counter: %u, EVR "
             "counter: %u ArrivalDiff: %u",
             lastTdcData->counter, lastEVRData->Counter,
             abs(duration_cast<milliseconds>(lastTdcData->arrivalTimestamp -
                                             lastEVRData->arrivalTimestamp)

                     .count()));
      lookingForNextTDC = false;
    } else {
      XTRACE(EVENT, DEB, "No TDC pair found for EVR: %u", lastEVRData->Counter);
    }
  } else {
    if (lastTdcData != NULL && lastTDCPair != lastTdcData) {
      XTRACE(EVENT, DEB, "Miss EVR pair for TDC arrived: %u, with counter: %u",
             lastTdcData->arrivalTimestamp.time_since_epoch(),
             lastTdcData->counter);

      statCounters.MissEVRPair++;
    }

    lastTdcData = std::make_unique<TDCDataEvent>(newTdcData);
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

  if (lastTdcData != nullptr) {

    if (isLastTimingDiffLowerThenThreshold()) {

      lastTDCPair = lastTdcData;
      lookingForNextTDC = false;
      statCounters.FoundEVRandTDCPairs++;

      XTRACE(EVENT, DEB,
             "I found TDC pair with previous TDC. TDC counter: %u, EVR "
             "counter: %u ArrivalDiff: %u",
             lastTdcData->counter, lastEVRData->Counter,
             abs(duration_cast<milliseconds>(lastTdcData->arrivalTimestamp -
                                             lastEVRData->arrivalTimestamp)
                     .count()));
    }
  }
}

uint64_t TimingEventHandler::getLastTDCTimestamp() const {
  if (lastTdcData != NULL) {
    return lastTdcData->tdcTimeStamp;
  } else {
    return 0;
  }
}

uint32_t TimingEventHandler::getTDCFrequency() const {
  return tdcRepetitionFrequency;
}

const std::shared_ptr<TDCDataEvent>
TimingEventHandler::getLastTdcEvent() const {
  return lastTdcData;
}

const std::shared_ptr<EVRDataEvent>
TimingEventHandler::getLastEvrEvent() const {
  return lastEVRData;
}

const shared_ptr<TDCDataEvent> TimingEventHandler::getLastTDCPair() const {
  return lastTDCPair;
}

void TimingEventHandler::updateTDCFrequency(const TDCDataEvent &newTdcData) {
  int64_t provision_missmatch = nextTDCTimeStamp - newTdcData.tdcTimeStamp;
  if (llabs(provision_missmatch) >= 15) {
    XTRACE(EVENT, DEB,
           "TDC provision miss! Current timestamp: %u, Expected timestamp: %u \
        Difference: %u",
           newTdcData.tdcTimeStamp, nextTDCTimeStamp,
           nextTDCTimeStamp - newTdcData.tdcTimeStamp);
  }

  uint32_t calcFrequency =
      lastTdcData != NULL ? newTdcData.tdcTimeStamp - lastTdcData->tdcTimeStamp
                          : DEFAULT_FREQUENCY_NS;

  if (calcFrequency != 0) {
    tdcRepetitionFrequency = calcFrequency;
  }
}

} // namespace Timepix3