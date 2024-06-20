// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief platform independent equivalent replacement of wrapper for the cheap and fast time stamp counter (TSC)
///
/// Uses the standard chrono library to provide a high resolution timer.
//===----------------------------------------------------------------------===//

#pragma once

#include <chrono>
#include <cstdint>

class CheckTimer {

public:
  /// Create a check timer without timeout value
  CheckTimer(void);

  /// Create a check timer with a timeout value in nanoseconds
  CheckTimer(uint64_t TimeOut);

  // Has timeout occurred? Then reset timer
  bool timeout(void);

  void reset(void); ///< record current time_point

  uint64_t timetsc(void); ///< time since T0

private:
  std::chrono::high_resolution_clock::time_point T0TimePoint;

  // If timeout not set use this 'infinite' value
  uint64_t TimeoutNS{0xffffffffffffffff};
};
