// Copyright (C) 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief container file to define main data transfer objects DTO's used for
//         timepix
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>

namespace Timepix3 {

struct TDCDataEvent {
    const uint8_t Type;
    const uint16_t TriggerCounter;
    const uint64_t Timestamp;
    const uint8_t Stamp;
    const uint64_t TdcTimeStamp;
    
    TDCDataEvent(uint8_t type, uint16_t triggerCounter, uint64_t timestamp, uint8_t stamp) :
    Type(type), TriggerCounter(triggerCounter), Timestamp(timestamp), Stamp(stamp), TdcTimeStamp(3.125 * timestamp + 0.26 * stamp) {}
};

  struct EVRDataEvent {
    const uint8_t Type;
    const uint8_t Unused;
    const uint16_t Unused2;
    const uint32_t Counter;
    const uint32_t PulseTimeSeconds;
    const uint32_t PulseTimeNanoSeconds;
    const uint32_t PrevPulseTimeSeconds;
    const uint32_t PrevPulseTimeNanoSeconds;

    EVRDataEvent(uint8_t type, uint8_t unused, uint16_t unused2, uint32_t counter, uint32_t pulseTimeSeconds, uint32_t pulseTimeNanoSeconds,
    uint32_t prevPulseTimeSeconds, uint32_t prevPulseTimeNanoSeconds) : Type(type), Unused(unused), Unused2(unused2), Counter(counter),
    PulseTimeSeconds(pulseTimeSeconds), PulseTimeNanoSeconds(pulseTimeNanoSeconds), PrevPulseTimeSeconds(prevPulseTimeSeconds), 
    PrevPulseTimeNanoSeconds(prevPulseTimeNanoSeconds) {}
    
  } __attribute__((__packed__));
  // not like above, the EVR readouts are structured like this so can be packed
  // and parsed this way

}