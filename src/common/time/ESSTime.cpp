// Copyright (C) 2019-2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of ESS time related methoods and classes
///
//===----------------------------------------------------------------------===//

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

#include <common/time/ESSTime.h>
#include <inttypes.h>

namespace esstime {

uint64_t ESSReferenceTime::setReference(const ESSTime &refESSTime) {
  TimeInNS = refESSTime.toNS();
  return TimeInNS.count();
}

uint64_t ESSReferenceTime::setPrevReference(const ESSTime &refPrevESSTime) {
  PrevTimeInNS = refPrevESSTime.toNS();
  return PrevTimeInNS.count();
}

void ESSReferenceTime::setMaxTOF(uint64_t NewMaxTOF) {
  MaxTOF = TimeDurationNano(NewMaxTOF);
}

uint64_t ESSReferenceTime::getTOF(ESSTime eventTime, uint32_t DelayNS) {
  TimeDurationNano timeval = eventTime.toNS() + TimeDurationNano(DelayNS);
  if (timeval < TimeInNS) {
    XTRACE(EVENT, WAR,
           "TOF negative: High: 0x%08x, Low: 0x%08x, timens %" PRIu64,
           ", PrevPTns: %" PRIu64, eventTime.getTimeHigh(),
           eventTime.getTimeLow(), timeval, TimeInNS);
    Stats.TofNegative++;
    return getPrevTOF(eventTime, DelayNS);
  }
  if ((timeval - TimeInNS) > MaxTOF) {
    XTRACE(EVENT, WAR, "High TOF: High: 0x%08x, Low: 0x%08x, timens %" PRIu64,
           ", PrevPTns: %" PRIu64, eventTime.getTimeHigh(),
           eventTime.getTimeLow(), timeval, TimeInNS);
    Stats.TofHigh++;
    return InvalidTOF;
  }
  Stats.TofCount++;
  return (timeval - TimeInNS).count();
}

/// \todo a valid value of TOF = 0 is in
uint64_t ESSReferenceTime::getPrevTOF(ESSTime eventTime, uint32_t DelayNS) {
  TimeDurationNano timeval = eventTime.toNS() + TimeDurationNano(DelayNS);
  if (timeval < PrevTimeInNS) {
    XTRACE(EVENT, WAR,
           "Prev TOF negative: High: 0x%04x, Low: 0x%04x, timens %" PRIu64,
           ", PrevPTns: %" PRIu64, eventTime.getTimeHigh(),
           eventTime.getTimeLow(), timeval, PrevTimeInNS);
    Stats.PrevTofNegative++;
    return InvalidTOF;
  }
  if ((timeval - PrevTimeInNS) > MaxTOF) {
    XTRACE(EVENT, WAR,
           "High Prev TOF: High: 0x%04x, Low: 0x%04x, timens %" PRIu64,
           ", PrevPTns: %" PRIu64, eventTime.getTimeHigh(),
           eventTime.getTimeLow(), timeval, PrevTimeInNS);
    Stats.PrevTofHigh++;
    return InvalidTOF;
  }
  Stats.PrevTofCount++;
  return (timeval - PrevTimeInNS).count();
}
} // namespace esstime
