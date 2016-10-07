/** Copyright (C) 2016 European Spallation Source */

#include <Detector.h>
#include <EFUArgs.h>
#include <Loader.h>
#include <Socket.h>
#include <Timer.h>
#include <cassert>
#include <thread>
#include <iostream>
#include <queue>
#include <stdio.h>
#include <unistd.h> // sleep()

static void input_thread(Loader *load, EFUArgs *args){
    load->detector->input_thread(args);
}

static void processing_thread(Loader *load, EFUArgs *args){
    load->detector->processing_thread(args);
}

static void output_thread(Loader *load, EFUArgs *args){
    load->detector->output_thread(args);
}

static void launch_thread(int lcore, void (*func)(Loader *, EFUArgs *), Loader * ld, EFUArgs * args)
{
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(lcore, &cpuset);

  std::thread * t = new std::thread(func, ld, args);
  t->join();

  int s = pthread_setaffinity_np(t->native_handle(), sizeof(cpu_set_t), &cpuset);
  assert(s != 0);
}
/**
 * Load detector, launch pipeline threads, then sleep forever
 */
int main(int argc, char *argv[]) {

  EFUArgs opts(argc, argv);

  std::cout << "Launching EFU as Instrument " << opts.det << std::endl;

  Loader dynamic(opts.det);
  if (dynamic.detector == NULL) {
    printf("Detector not loadable, exiting..\n");
    exit(1);
  }

  launch_thread(12, input_thread, &dynamic, &opts);
  #if 0
  std::thread t1(input_thread, &dynamic, &opts);
  t1.join();
  std::thread t2(output_thread, &dynamic, &opts);
  t2.join();
  std::thread t3(processing_thread, &dynamic, &opts);
  t3.join();
  #endif

  Timer now;
  while (1) {
    sleep(2);
  }
  return 0;
}
