/// Copyright (C) 2016-2018 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// wrapper for the cheap and fast time stamp counter (TSC) which is a 64 bit
/// counter running at CPU clock. Can be used (with caution) as a timer
///
//===----------------------------------------------------------------------===//

#include <cstdint>
#include <libs/include/gccintel.h>

class TSCTimer {

public:
  TSCTimer(void);

  void now(void); /**< record current time_point */

  uint64_t timetsc(void); /**< time since t1 */

private:
  uint64_t t1;
};
