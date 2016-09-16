#include <Thread.h>
#include <iostream>
#include <pthread.h>

Thread::Thread(int lcore, void (*func)(void)) : lcore_(lcore) {
  t_ = std::thread(func);
   SetAffinity(lcore);
}


void Thread::SetAffinity(int lcore) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(lcore, &cpuset);

  int s =
      pthread_setaffinity_np(t_.native_handle(), sizeof(cpu_set_t), &cpuset);
  if (s != 0) {
    std::cout << "thread affinity error" << std::endl;
    exit(1);
  }

}
