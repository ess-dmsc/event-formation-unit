/** Copyright (C) 2016 European Spallation Source */

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
