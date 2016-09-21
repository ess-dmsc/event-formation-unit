#include <Timer.h>

/** */
Timer::Timer(void) {
  timerclear(&mTvStart);
  timerclear(&mTvStop);
}

/** */
void Timer::start(void) { gettimeofday(&mTvStart, 0); }

/** */
void Timer::stop(void) { gettimeofday(&mTvStop, 0); }

uint64_t Timer::timeus(void) {
  struct timeval res;
  timersub(&mTvStop, &mTvStart, &res);
  return res.tv_sec * 1000000 + res.tv_usec;
}
