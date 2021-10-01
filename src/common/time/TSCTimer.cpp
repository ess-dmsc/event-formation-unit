// Copyright (C) 2016-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation (\todo put in header?
//===----------------------------------------------------------------------===//


#include <common/time/TSCTimer.h>

///
TSCTimer::TSCTimer(void) {
  T0 = rdtsc();
}

///
TSCTimer::TSCTimer(uint64_t Timeout)
  : TimeoutTicks(Timeout) {
  T0 = rdtsc();
}

/// Determine if a timeout has occured and reset timer
bool TSCTimer::timeout(void) {
  if (timetsc() >= TimeoutTicks) {
    reset();
    return true;
  }
  return false;
}

///
void TSCTimer::reset(void) {
  T0 = rdtsc();
}

///
uint64_t TSCTimer::timetsc(void) {
  return (rdtsc() - T0);
}
