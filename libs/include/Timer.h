/** Copyright (C) 2016 European Spallation Source */

#include <chrono>
#include <cstdint>

class Timer {

  typedef std::chrono::high_resolution_clock HRClock;
  typedef std::chrono::time_point<HRClock> TP;

public:
  Timer(void);

  void now(void); /**< record current time_point */

  uint64_t timeus(void); /**< time since tp */

private:
  TP t1;
};
