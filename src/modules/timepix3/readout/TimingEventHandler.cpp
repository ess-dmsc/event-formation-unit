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

#include <common/debug/Trace.h>
#include <readout/TimingEventHandler.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

using namespace chrono;
using namespace efutils;

///\todo: This should come from configuration file
const  uint32_t TimingEventHandler::DEFAULT_FREQUENCY_NS =
    hzToNanoseconds(14).count();

void TimingEventHandler::applyData(const shared_ptr<TDCDataEvent> &newTdcData) {

  XTRACE(EVENT, DEB,
         "New TDC Data with Timestamp: %u, Frequency: %u, arrival: %u",
         lastTDCData->tdcTimeStamp, tdcRepetitionFrequency,
         lastTDCData->arrivalTimestamp.time_since_epoch());

  if (lastTDCData != nullptr &&
      newTdcData->counter != lastTDCData->counter + 1) {
    statCounters.MissTDCCounter += newTdcData->counter - lastTDCData->counter + 1;
  }

  lastTDCData = newTdcData;

  statCounters.TDCTimeStampReadout++;

  if (isLastTimingDiffLowerThenThreshold()) {
    epochESSPulseTimeObservable.publishData(EpochESSPulseTime(lastEVRData->pulseTimeSeconds,
                              lastEVRData->pulseTimeNanoSeconds, *lastTDCData));

    statCounters.EVRPairFound++;
  }
}

void TimingEventHandler::applyData(const shared_ptr<EVRDataEvent> &newEVRData) {

  XTRACE(EVENT, DEB, "New EVR Data with PulseTime, s: %u, ns: %u, arrival: %u",
         lastEVRData->pulseTimeSeconds, lastEVRData->pulseTimeNanoSeconds,
         lastEVRData->arrivalTimestamp.time_since_epoch());

  if (lastEVRData != nullptr &&
      newEVRData->counter != lastEVRData->counter + 1) {
    statCounters.MissEVRCounter += newEVRData->counter - lastEVRData->counter +1;
  }

  lastEVRData = newEVRData;

  statCounters.EVRTimeStampReadouts++;

  if (isLastTimingDiffLowerThenThreshold()) {
    epochESSPulseTimeObservable.publishData(EpochESSPulseTime(lastEVRData->pulseTimeSeconds,
                              lastEVRData->pulseTimeNanoSeconds, *lastTDCData));

    statCounters.TDCPairFound++;
  }
}

} // namespace Timepix3