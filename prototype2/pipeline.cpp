/** Copyright (C) 2016 European Spallation Source */

#include <Detector.h>
#include <EFUArgs.h>
#include <Loader.h>
#include <cassert>
#include <thread>
#include <iostream>
#include <queue>
#include <stdio.h>
#include <unistd.h> // sleep()

static void input_thread(Loader *load, EFUArgs *args){
    load->detector->input_thread(NULL);
}

static void processing_thread(Loader *load, EFUArgs *args){
    load->detector->processing_thread(NULL);
}

static void output_thread(Loader *load, EFUArgs *args){
    load->detector->output_thread(NULL);
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

  std::thread t1(input_thread, &dynamic, &opts);
  t1.join();
  std::thread t2(output_thread, &dynamic, &opts);
  t2.join();
  std::thread t3(processing_thread, &dynamic, &opts);
  t3.join();

  while (1) {
    sleep(2);
  }
  return 0;
}
