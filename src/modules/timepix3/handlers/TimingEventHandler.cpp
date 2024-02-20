// Copyright (C) 2023-2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation for TDC and EVR timing event observers. Responsible
//         to follow up different timing events and calculate the global time
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <handlers/TimingEventHandler.h>
#include <memory>
#include <utility>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

using namespace timepixDTO;
using namespace timepixReadout;

///\todo: This should come from configuration file
const uint32_t TimingEventHandler::DEFAULT_FREQUENCY_NS =
    efutils::hzToNanoseconds(14).count();

void TimingEventHandler::applyData(const TDCReadout &tdcReadout) {

  XTRACE(EVENT, DEB,
         "New TDC Data with Timestamp: %u, Frequency: %u, arrival: %u",
         lastTDCData->tdcTimeStamp, tdcRepetitionFrequency,
         lastTDCData->arrivalTimestamp.time_since_epoch());

  if (lastTDCData != nullptr) {
    if (tdcReadout.counter > lastTDCData->counter + 1) {
      statCounters.MissTDCCounter +=
          tdcReadout.counter - (lastTDCData->counter + 1);
    } else if (tdcReadout.counter < lastTDCData->counter &&
               tdcReadout.counter != 1) {
      return;
    } else if (tdcReadout.counter == lastTDCData->counter) {
      return;
    }
  }

  lastTDCData = std::make_unique<TDCDataEvent>(
      tdcReadout.counter, tdcReadout.timestamp, tdcReadout.stamp);

  if (isLastTimingDiffLowerThenThreshold()) {
    statCounters.EVRPairFound++;
    statCounters.ESSGlobalTimeCounter++;
    publishData(ESSGlobalTimeStamp(lastEVRData->pulseTimeInEpochNs,
                                   lastTDCData->tdcTimeInPixelClock));
  }
}

void TimingEventHandler::applyData(const EVRReadout &evrReadout) {

  XTRACE(EVENT, DEB, "New EVR Data with PulseTime, s: %u, ns: %u, arrival: %u",
         lastEVRData->pulseTimeInEpochNs,
         lastEVRData->arrivalTimestamp.time_since_epoch());

  if (lastEVRData != nullptr) {
    if (evrReadout.counter > lastEVRData->counter + 1) {
      statCounters.MissEVRCounter +=
          evrReadout.counter - (lastEVRData->counter + 1);
    } else if (evrReadout.counter < lastEVRData->counter &&
               evrReadout.counter != 1) {
      return;
    } else if (evrReadout.counter == lastEVRData->counter) {
      return;
    }
  }

  lastEVRData = std::make_unique<EVRDataEvent>(evrReadout.counter,
                                               evrReadout.pulseTimeSeconds,
                                               evrReadout.pulseTimeNanoSeconds);

  if (isLastTimingDiffLowerThenThreshold()) {
    statCounters.TDCPairFound++;
    statCounters.ESSGlobalTimeCounter++;
    publishData(ESSGlobalTimeStamp(lastEVRData->pulseTimeInEpochNs,
                                   lastTDCData->tdcTimeInPixelClock));
  }
}

} // namespace Timepix3