#include <inttypes.h>
#include <sys/time.h>

class Timer {
public:
  Timer();
  void start(void);
  void stop(void);
  uint64_t timeus(void);

private:
  struct timeval mTvStart, mTvStop;
};
