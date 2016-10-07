/** Copyright (C) 2016 European Spallation Source */

#include <Detector.h>
#include <Launcher.h>
#include <cassert>
#include <thread>

void Launcher::input_thread(Loader *load, EFUArgs *args) {
  load->detector->input_thread(args);
}

void Launcher::processing_thread(Loader *load, EFUArgs *args) {
  load->detector->processing_thread(args);
}

void Launcher::output_thread(Loader *load, EFUArgs *args) {
  load->detector->output_thread(args);
}

void Launcher::launch(int lcore, void (*func)(Loader *, EFUArgs *), Loader *ld,
                      EFUArgs *args) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(lcore, &cpuset);

  std::thread *t = new std::thread(func, ld, args);

  int s =
      pthread_setaffinity_np(t->native_handle(), sizeof(cpu_set_t), &cpuset);
  assert(s == 0);
  t->join();
}

Launcher::Launcher(Loader *dynamic, EFUArgs *args, int input, int processing,
                   int output) {
  launch(input, input_thread, dynamic, args);
  launch(output, output_thread, dynamic, args);
  launch(processing, processing_thread, dynamic, args);
}
