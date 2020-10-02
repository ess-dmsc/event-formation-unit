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

class ESSTime {
public:
  // ESS clock is 88052500 Hz
  const double NsPerTick{11.356860963629653};
  const uint64_t OneBillion{1000000000LU};

  /// \brief save reference (pulse) time
  uint64_t setReference(uint32_t High, uint32_t Low) {
    TimeInNS =toNS(High, Low);
    return TimeInNS;
  }

  /// \brief calculate TOF from saved reference and current time
  uint64_t getTOF(uint32_t High, uint32_t Low) {
    return toNS(High, Low) - TimeInNS;
  }
private:
  /// \brief convert ess High/Low time to NS
  uint64_t toNS(uint32_t High, uint32_t Low) {
    assert(Low < 88052500);
    return  High * OneBillion + Low * NsPerTick;
  }
  uint64_t TimeInNS{0};
};
