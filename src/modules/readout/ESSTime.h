// Copyright (C) 2019-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief ESS HighTime/LowTime handler class
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <cassert>

struct Stats_t {
  uint64_t TofCount;
  uint64_t TofNegative;
  uint64_t PrevTofCount;
  uint64_t PrevTofNegative;
};

class ESSTime {
public:
  // ESS clock is 88052500 Hz
  const double NsPerTick{11.356860963629653};
  const uint64_t OneBillion{1000000000LU};

  /// \brief save reference (pulse) time
  uint64_t setReference(uint32_t High, uint32_t Low) {
    TimeInNS = toNS(High, Low);
    return TimeInNS;
  }

  uint64_t setPrevReference(uint32_t PrevHigh, uint32_t PrevLow) {
    PrevTimeInNS = toNS(PrevHigh, PrevLow);
    return PrevTimeInNS;
  }

  /// \brief calculate TOF from saved reference and current time
  uint64_t getTOF(uint32_t High, uint32_t Low, uint32_t DelayNS = 0) {
    uint64_t timeval = toNS(High, Low) + DelayNS;
    if (timeval < TimeInNS) {
      Stats.TofNegative++;
      return 0xFFFFFFFFFFFFFFFFULL;
    }
    Stats.TofCount++;
    return timeval - TimeInNS;
  }

  /// \brief calculate TOF from saved reference and current time
  /// \todo a valid value of TOF = 0 is in
  uint64_t getPrevTOF(uint32_t High, uint32_t Low, uint32_t DelayNS = 0) {
    uint64_t timeval = toNS(High, Low) + DelayNS;
    if (timeval < PrevTimeInNS) {
      Stats.PrevTofNegative++;
      return 0xFFFFFFFFFFFFFFFFULL;
    }
    Stats.PrevTofCount++;
    return timeval - PrevTimeInNS;
  }


  /// \brief convert ess High/Low time to NS
  uint64_t toNS(uint32_t High, uint32_t Low) {
    //assert(Low < 88052500);
    return  High * OneBillion + Low * NsPerTick;
  }
public:
  struct Stats_t Stats = {};
private:
  uint64_t TimeInNS{0};
  uint64_t PrevTimeInNS{0};
};
