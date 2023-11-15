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
#include <cstdint>
#include <readout/TimingEventHandler.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

void TimingEventHandler::applyData(const TDCDataEvent &newData) {

  int64_t provision_missmatch = nextTDCTimeStamp - newData.TdcTimeStamp;
  if (llabs(provision_missmatch) >= 15) {
    XTRACE(EVENT, DEB,
           "TDC provision miss! Current timestamp: %u, Expected timestamp: %u \
        Difference: %u",
           newData.TdcTimeStamp, nextTDCTimeStamp,
           nextTDCTimeStamp - newData.TdcTimeStamp);
  }

  frequency = lastTdcFrame != NULL
                  ? newData.TdcTimeStamp - lastTdcFrame->TdcTimeStamp
                  : 0;
  nextTDCTimeStamp = newData.TdcTimeStamp + frequency;

  lastTdcFrame = std::make_unique<TDCDataEvent>(newData);

  XTRACE(EVENT, DEB, "New TDC Data with Timestamp: %u, Frequency: %u",
         lastTdcFrame->TdcTimeStamp, frequency);
}

uint64_t TimingEventHandler::getLastTDCTimestamp() {
  if (lastTdcFrame != NULL) {
    return lastTdcFrame->TdcTimeStamp;
  } else {
    return 0;
  }
}

uint32_t TimingEventHandler::getTDCFrequency() const { return frequency; }

const std::shared_ptr<TDCDataEvent>
TimingEventHandler::getLastTdcEvent() const {
  return lastTdcFrame;
}

} // namespace Timepix3