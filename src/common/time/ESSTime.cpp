// Copyright (C) 2019 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of ESS time related methods and classes
///
//===----------------------------------------------------------------------===//

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

#include <common/Statistics.h>
#include <common/time/ESSTime.h>
#include <inttypes.h>
#include <optional>

namespace esstime {

uint64_t ESSReferenceTime::setReference(const ESSTime &refESSTime) {
  TimeInNS = refESSTime.toNS();
  return TimeInNS.count();
}

uint64_t ESSReferenceTime::setReference(uint64_t refTime) {
  TimeInNS = TimeDurationNano(refTime);
  return TimeInNS.count();
}

uint64_t ESSReferenceTime::setPrevReference(const ESSTime &refPrevESSTime) {
  PrevTimeInNS = refPrevESSTime.toNS();
  return PrevTimeInNS.count();
}

void ESSReferenceTime::setMaxTOF(uint64_t NewMaxTOF) {
  MaxTOF = TimeDurationNano(NewMaxTOF);
}

tof_t ESSReferenceTime::getTOF(const ESSTime &eventTime, uint32_t DelayNS) {
  TimeDurationNano timeval = eventTime.toNS() + TimeDurationNano(DelayNS);
  if (timeval < TimeInNS) {
    XTRACE(EVENT, WAR,
           "TOF negative: High: 0x%08x, Low: 0x%08x, timens %" PRIu64,
           ", PrevPTns: %" PRIu64, eventTime.getTimeHigh(),
           eventTime.getTimeLow(), timeval, TimeInNS);
    Counters.TofNegative++;
    return getPrevTOF(eventTime, DelayNS);
  }
  if ((timeval - TimeInNS) > MaxTOF) {
    XTRACE(EVENT, WAR, "High TOF: High: 0x%08x, Low: 0x%08x, timens %" PRIu64,
           ", PrevPTns: %" PRIu64, eventTime.getTimeHigh(),
           eventTime.getTimeLow(), timeval, TimeInNS);
    Counters.TofHigh++;
    return InvalidTof;
  }

  Counters.TofCount++;
  return (timeval - TimeInNS).count();
}

tof_t ESSReferenceTime::getPrevTOF(const ESSTime &eventTime,
                                      uint32_t DelayNS) {
  TimeDurationNano timeval = eventTime.toNS() + TimeDurationNano(DelayNS);
  if (timeval < PrevTimeInNS) {
    XTRACE(EVENT, WAR,
           "Prev TOF negative: High: 0x%08x, Low: 0x%08x, timens %" PRIu64,
           ", PrevPTns: %" PRIu64, eventTime.getTimeHigh(),
           eventTime.getTimeLow(), timeval, PrevTimeInNS);
    Counters.PrevTofNegative++;
    return InvalidTof;
  }
  if ((timeval - PrevTimeInNS) > MaxTOF) {
    XTRACE(EVENT, WAR,
           "High Prev TOF: High: 0x%08x, Low: 0x%08x, timens %" PRIu64,
           ", PrevPTns: %" PRIu64, eventTime.getTimeHigh(),
           eventTime.getTimeLow(), timeval, PrevTimeInNS);
    Counters.PrevTofHigh++;
    return InvalidTof;
  }
  Counters.PrevTofCount++;
  return (timeval - PrevTimeInNS).count();
}

} // namespace esstime
