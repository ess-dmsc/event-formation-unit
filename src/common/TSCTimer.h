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

#include <cstdint>
#include <common/gccintel.h>

class TSCTimer {

public:
  TSCTimer(void);

  void now(void); ///< record current time_point

  uint64_t timetsc(void); ///< time since t1

private:
  uint64_t t1;
};
