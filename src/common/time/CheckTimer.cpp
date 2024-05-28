// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of the check timer class which uses the standard chrono library to provide a high resolution timer.
//===----------------------------------------------------------------------===//

#include <common/time/CheckTimer.h>

///
CheckTimer::CheckTimer(void) {
  reset();
}

/// \param Timeout
CheckTimer::CheckTimer(uint64_t Timeout) : TimeoutNS(Timeout) {
  reset();
}

/// Determine if a timeout has occured and reset timer
bool CheckTimer::timeout(void) {
  if (timetsc() >= TimeoutNS) {
    reset();
    return true;
  }
  return false;
}

///
void CheckTimer::reset(void) {
  T0TimePoint = std::chrono::high_resolution_clock::now();
}

///
uint64_t CheckTimer::timetsc(void) {
  auto Now = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(Now - T0TimePoint).count();
}
