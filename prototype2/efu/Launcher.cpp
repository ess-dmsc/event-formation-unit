/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cassert>
#include <common/Detector.h>
#include <common/EFUArgs.h>
#include <efu/Launcher.h>
#include <iostream>
#include <thread>

using namespace std;

/** Can't call detector threads directly from std:thread as
 *  they are virtual functions, so need to add one step.
 */
void Launcher::input_thread(Loader *load) { load->detector->input_thread(); }

void Launcher::processing_thread(Loader *load) {
  load->detector->processing_thread();
}

void Launcher::output_thread(Loader *load) { load->detector->output_thread(); }

/** Create a thread 'func()', set its cpu affinity and calls join() */
void Launcher::launch(int __attribute__((unused)) lcore, void (*func)(Loader *),
                      Loader *ld) {
#ifdef __linux__
  std::thread *t =
#endif
      new std::thread(func, ld);

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
#pragma message("setaffinity only implemented for Linux")
#endif
}

Launcher::Launcher(Loader *dynamic, std::vector<int> &cpus) {
  if (dynamic->detector == nullptr) {
    cout << "Detector not loadable, no processing ..." << endl;
    return;
  }

  launch(cpus[0], input_thread, dynamic);
  launch(cpus[2], output_thread, dynamic);
  launch(cpus[1], processing_thread, dynamic);
}
