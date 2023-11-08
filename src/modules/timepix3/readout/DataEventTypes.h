// Copyright (C) 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief container file for all data event structs
//===----------------------------------------------------------------------===//

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

}