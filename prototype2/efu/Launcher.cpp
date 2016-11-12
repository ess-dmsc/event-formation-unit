/** Copyright (C) 2016 European Spallation Source ERIC */

#include <cassert>
#include <common/Detector.h>
#include <efu/Launcher.h>
#include <iostream>
#include <thread>

using namespace std;

/** Can't call detector threads directly from std:thread as
 *  they are virtual functions, so need to add one step.
 */
void Launcher::input_thread(Loader *load, EFUArgs *args) {
  load->detector->input_thread(args);
}

void Launcher::processing_thread(Loader *load, EFUArgs *args) {
  load->detector->processing_thread(args);
}

void Launcher::output_thread(Loader *load, EFUArgs *args) {
  load->detector->output_thread(args);
}

/** Create a thread 'func()', set its cpu affinity and calls join() */
void Launcher::launch(int __attribute__((unused)) lcore, void (*func)(Loader *, EFUArgs *), Loader *ld,
                      EFUArgs *args) {
#ifdef __linux__
  std::thread *t =
#endif
new std::thread(func, ld, args);

#ifdef __linux__
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(lcore, &cpuset);
#ifndef NDEBUG
  int s =
#endif
      pthread_setaffinity_np(t->native_handle(), sizeof(cpu_set_t), &cpuset);
  assert(s == 0);
#else
#pragma message ("setaffinity only implemented for Linux")
#endif
}

Launcher::Launcher(Loader *dynamic, EFUArgs *args, std::vector<int>& cpus) {
  if (dynamic->detector == nullptr) {
    cout << "Detector not loadable, no processing ..." << endl;
    return;
  }

  launch(cpus[0], input_thread, dynamic, args);
  launch(cpus[2], output_thread, dynamic, args);
  launch(cpus[1], processing_thread, dynamic, args);
}
