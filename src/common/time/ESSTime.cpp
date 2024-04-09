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

namespace esstime {

/// \brief save reference (pulse) time
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

/// \brief calculate TOF from saved reference and current event time
uint64_t ESSReferenceTime::getTOF(ESSTime eventEssTime, uint32_t DelayNS) {
  TimeDurationNano timeval = eventEssTime.toNS() + TimeDurationNano(DelayNS);
  if (timeval < TimeInNS) {
    XTRACE(EVENT, WAR,
           "TOF negative: High: 0x%08x, Low: 0x%08x, timens %" PRIu64,
           ", PrevPTns: %" PRIu64, eventEssTime.getTimeHigh(),
           eventEssTime.getTimeLow(), timeval, TimeInNS);
    Stats.TofNegative++;
    return getPrevTOF(eventEssTime, DelayNS);
  }
  if ((timeval - TimeInNS) > MaxTOF) {
    XTRACE(EVENT, WAR, "High TOF: High: 0x%08x, Low: 0x%08x, timens %" PRIu64,
           ", PrevPTns: %" PRIu64, eventEssTime.getTimeHigh(),
           eventEssTime.getTimeLow(), timeval, TimeInNS);
    Stats.TofHigh++;
    return InvalidTOF;
  }
  Stats.TofCount++;
  return (timeval - TimeInNS).count();
}

/// \brief calculate TOF from saved reference and current event time
/// \todo a valid value of TOF = 0 is in
uint64_t ESSReferenceTime::getPrevTOF(ESSTime eventEssTime, uint32_t DelayNS) {
  TimeDurationNano timeval = eventEssTime.toNS() + TimeDurationNano(DelayNS);
  if (timeval < PrevTimeInNS) {
    XTRACE(EVENT, WAR,
           "Prev TOF negative: High: 0x%04x, Low: 0x%04x, timens %" PRIu64,
           ", PrevPTns: %" PRIu64, eventEssTime.getTimeHigh(),
           eventEssTime.getTimeLow(), timeval, PrevTimeInNS);
    Stats.PrevTofNegative++;
    return InvalidTOF;
  }
  if ((timeval - PrevTimeInNS) > MaxTOF) {
    XTRACE(EVENT, WAR,
           "High Prev TOF: High: 0x%04x, Low: 0x%04x, timens %" PRIu64,
           ", PrevPTns: %" PRIu64, eventEssTime.getTimeHigh(),
           eventEssTime.getTimeLow(), timeval, PrevTimeInNS);
    Stats.PrevTofHigh++;
    return InvalidTOF;
  }
  Stats.PrevTofCount++;
  return (timeval - PrevTimeInNS).count();
}
} // namespace esstime
