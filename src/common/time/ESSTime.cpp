// Copyright (C) 2019 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of ESS time related methods and classes
///
//===----------------------------------------------------------------------===//

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

#include <common/Statistics.h>
#include <common/time/ESSTime.h>

#include <chrono>
#include <ctime>
#include <inttypes.h>
#include <iomanip>
#include <optional>

namespace esstime {

std::string toString(TimeDurationNano TimeStamp)
{
    using namespace std::chrono;

    // Split into seconds + milliseconds
    system_clock::time_point tp{nanoseconds{TimeStamp}};
    auto seconds = time_point_cast<std::chrono::seconds>(tp);
    auto ms = duration_cast<milliseconds>(tp - seconds).count();

    // Convert seconds to tm
    std::time_t tt = system_clock::to_time_t(seconds);
    std::tm tm{};
    localtime_r(&tt, &tm);

    // Format string
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
        << '.' << std::setw(3) << std::setfill('0') << ms;

    return oss.str();
}  

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
  const std::string t0 = toString(timeval);
  const std::string t1 = toString(TimeInNS);

  if (timeval < TimeInNS) {
    XTRACE(EVENT, WAR,
           "TOF negative:      High: 0x%08x, Low: 0x%08x, PT/ns: %" PRIu64 " (%s), Prev PT/ns: %" PRIu64 " (%s)", 
           eventTime.getTimeHigh(), eventTime.getTimeLow(), timeval, t0.c_str(), TimeInNS, t1.c_str());
    Counters.TofNegative++;
    return getPrevTOF(eventTime, DelayNS);
  }

  if ((timeval - TimeInNS) > MaxTOF) {
    XTRACE(EVENT, WAR, 
           "High TOF:          High: 0x%08x, Low: 0x%08x, PT/ns: %" PRIu64 " (%s), Prev PT/ns: %" PRIu64 " (%s)", 
           eventTime.getTimeHigh(), eventTime.getTimeLow(), timeval, t0.c_str(), TimeInNS, t1.c_str());
    Counters.TofHigh++;
    return InvalidTof;
  }

  Counters.TofCount++;
  return (timeval - TimeInNS).count();
}

tof_t ESSReferenceTime::getPrevTOF(const ESSTime &eventTime,
                                      uint32_t DelayNS) {
  TimeDurationNano timeval = eventTime.toNS() + TimeDurationNano(DelayNS);
  const std::string t0 = toString(timeval);
  const std::string t1 = toString(PrevTimeInNS);

  if (timeval < PrevTimeInNS) {
    XTRACE(EVENT, WAR,
           "Prev TOF negative: High: 0x%08x, Low: 0x%08x, PT/ns: %" PRIu64 " (%s), Prev PT/ns: %" PRIu64 " (%s)",
           eventTime.getTimeHigh(), eventTime.getTimeLow(), timeval, t0.c_str(), PrevTimeInNS, t1.c_str());
    Counters.PrevTofNegative++;
    return InvalidTof;
  }

  if ((timeval - PrevTimeInNS) > MaxTOF) {
    XTRACE(EVENT, WAR,
           "High Prev TOF:     High: 0x%08x, Low: 0x%08x, PT/ns: %" PRIu64 " (%s), Prev PT/ns: %" PRIu64 " (%s)", 
           eventTime.getTimeHigh(), eventTime.getTimeLow(), timeval, t0.c_str(), PrevTimeInNS, t1.c_str());
    Counters.PrevTofHigh++;
    return InvalidTof;
  }
  Counters.PrevTofCount++;

  return (timeval - PrevTimeInNS).count();
}

} // namespace esstime
