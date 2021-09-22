// Copyright (C) 2019-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief ESS HighTime/LowTime handler class
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cassert>
#include <cinttypes>
#include <common/Trace.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

class ESSTime {
public:
  struct Stats_t {
    int64_t TofCount;
    int64_t TofNegative;
    int64_t PrevTofCount;
    int64_t PrevTofNegative;
  };

  // ESS clock is 88052500 Hz
  const double NsPerTick{11.356860963629653};
  const uint64_t OneBillion{1000000000LU};
  const uint64_t InvalidTOF{0xFFFFFFFFFFFFFFFFULL};

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
      return InvalidTOF;
    }
    Stats.TofCount++;
    return timeval - TimeInNS;
  }

  /// \brief calculate TOF from saved reference and current time
  /// \todo a valid value of TOF = 0 is in
  uint64_t getPrevTOF(uint32_t High, uint32_t Low, uint32_t DelayNS = 0) {
    uint64_t timeval = toNS(High, Low) + DelayNS;
    if (timeval < PrevTimeInNS) {
      XTRACE(EVENT, WAR,
              "Prev TOF negative: High: 0x%04x, Low: 0x%04x, timens %" PRIu64, ", PrevPTns: %" PRIu64,
              High, Low, timeval, PrevTimeInNS);
      Stats.PrevTofNegative++;
      return InvalidTOF;
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
  uint64_t TimeInNS{0};
  uint64_t PrevTimeInNS{0};
};
