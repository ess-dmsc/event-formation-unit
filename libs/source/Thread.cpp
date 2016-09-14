#include <Thread.h>
#include <iostream>
#include <pthread.h>

Thread::Thread(int cpu, void (*func)(void)) : cpu_(cpu) {
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
