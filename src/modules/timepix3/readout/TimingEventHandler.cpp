// Copyright (C) 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Listener interface for data events
//===----------------------------------------------------------------------===//

#include <readout/TimingEventHandler.h>
#include <common/debug/Trace.h>
#include <cstdint>


namespace Timepix3 {

void TimingEventHandler::applyData(const TDCData& newData) {

    int64_t provision_missmatch = nextTDCTimeStamp - newData.TdcTimeStamp;
    if(llabs(provision_missmatch) >= 15) {
        XTRACE(EVENT, DEB, "TDC provision miss! Current timestamp: %u, Expected timestamp: %u \
        Difference: %u",
        newData.TdcTimeStamp, nextTDCTimeStamp, nextTDCTimeStamp - newData.TdcTimeStamp);
    }

    frequency = lastTdcFrame != NULL? newData.TdcTimeStamp - lastTdcFrame->TdcTimeStamp : 0;
    nextTDCTimeStamp = newData.TdcTimeStamp + frequency;
    
    lastTdcFrame = std::make_unique<TDCData>(newData);
    
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

uint32_t TimingEventHandler::getTDCFrequency() const {
    return frequency;
}

}