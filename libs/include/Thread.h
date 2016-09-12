#include <iostream>
#include <pthread.h>
#include <thread>

class Thread {
public:
  Thread(int cpu, void (*func)(void)) : cpu_(cpu) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_, &cpuset);

    t_ = std::thread(func);
    int s =
        pthread_setaffinity_np(t_.native_handle(), sizeof(cpu_set_t), &cpuset);
    if (s != 0) {
      std::cout << "thread affinity error" << std::endl;
      exit(1);
    }
  }

private:
  int cpu_;
  std::thread t_;
};
