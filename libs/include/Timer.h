#include <sys/time.h>
#include <inttypes.h>

class Timer {
 public:
  Timer();
  void Start(void);
  void Stop(void);
  uint64_t ElapsedUS(void);
 private:
  struct timeval mTvStart, mTvStop;
};
