/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cassert>
#include <common/Detector.h>
#include <common/EFUArgs.h>
#include <common/Trace.h>
#include <efu/Launcher.h>
#include <iostream>
#include <thread>

/** Can't call detector threads directly from std:thread as
 *  they are virtual functions, so need to add one step.
 */
void Launcher::input_thread(Detector *detector) {
  GLOG_INF("Launching input thread");
  detector->input_thread();
}

void Launcher::processing_thread(Detector *detector) {
  GLOG_INF("Launching processing thread");
  detector->processing_thread();
}

void Launcher::output_thread(Detector *detector) {
  GLOG_INF("Launching output thread");
  detector->output_thread();
}

/** Create a thread 'func()', set its cpu affinity and calls join() */
void Launcher::launch(int __attribute__((unused)) lcore, void (*func)(Detector *),
                      Detector *detector) {

XTRACE(MAIN, ALW, "Creating new thread (lcore %d)\n", lcore);
#ifdef __linux__
  std::thread *t = new std::thread(func, detector);
#else
  new std::thread(func, detector);
#endif

#ifdef __linux__
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(lcore, &cpuset);
  if (lcore >= 0) {
    XTRACE(MAIN, ALW, "Setting thread affinity to core %d\n", lcore);
    GLOG_INF("Setting thread affinity to core " + std::to_string(lcore));

    int __attribute__((unused))s = pthread_setaffinity_np(t->native_handle(), sizeof(cpu_set_t), &cpuset);
    assert(s == 0);
  }
#else
#pragma message("setaffinity only implemented for Linux")
  GLOG_WAR("setaffinity only implemented for Linux");
#endif
}

Launcher::Launcher(Detector *detector, std::vector<int> &cpus) {
  if (detector == nullptr) {
    GLOG_CRI("Detector not loadable, no processing ...");
    XTRACE(MAIN, CRI, "Detector not loadable, no processing ...\n");
    return;
  }

  launch(cpus[0], input_thread, detector);
  launch(cpus[2], output_thread, detector);
  launch(cpus[1], processing_thread, detector);
}
