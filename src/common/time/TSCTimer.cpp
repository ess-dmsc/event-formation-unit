// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <common/time/TSCTimer.h>
#include <common/system/intel.h>

TSCTimer::TSCTimer(uint64_t Timeout)
  : TimeOutTicks(Timeout) {
  T0 = rdtsc();
}

bool TSCTimer::timeout() {
  if (timeTSC() >= TimeOutTicks) {
    reset();
    return true;
  }

  return false;
}

void TSCTimer::reset() {
  T0 = rdtsc();
}

uint64_t TSCTimer::timeTSC() {
  return (rdtsc() - T0);
}
