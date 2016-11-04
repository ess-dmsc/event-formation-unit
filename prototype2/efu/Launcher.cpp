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
void Launcher::launch(int lcore, void (*func)(Loader *, EFUArgs *), Loader *ld,
                      EFUArgs *args) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(lcore, &cpuset);

  std::thread *t = new std::thread(func, ld, args);

#ifndef NDEBUG
  int s =
#endif
      pthread_setaffinity_np(t->native_handle(), sizeof(cpu_set_t), &cpuset);
  assert(s == 0);
}

Launcher::Launcher(Loader *dynamic, EFUArgs *args, int input, int processing,
                   int output) {
  if (dynamic->detector == nullptr) {
    cout << "Detector not loadable, no processing ..." << endl;
    return;
  }
  launch(input, input_thread, dynamic, args);
  launch(output, output_thread, dynamic, args);
  launch(processing, processing_thread, dynamic, args);
}
