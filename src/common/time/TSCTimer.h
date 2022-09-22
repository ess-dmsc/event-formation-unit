// Copyright (C) 2016-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief wrapper for the cheap and fast time stamp counter (TSC)
///
/// TSC is a 64 bit counter running at CPU clock. Can be used (with caution)
/// as a high resolution timer.
//===----------------------------------------------------------------------===//

#include <common/system/gccintel.h>
#include <cstdint>

class TSCTimer {

public:
  /// Create a TSC timer without timeout value
  TSCTimer(void);

  /// Create a TSC timer with a timeout value
  TSCTimer(uint64_t TimeOut);

  // Has timeout occured? Then reset timer
  bool timeout(void);

  void reset(void); ///< record current time_point

  uint64_t timetsc(void); ///< time since T0

private:
  uint64_t T0; ///< reference tsc timestamp

  // If timeout not set use this 'infinite' value
  uint64_t TimeoutTicks{0xffffffffffffffff};
};
