/** Copyright (C) 2016 European Spallation Source */

#include <EFUArgs.h>
#include <Launcher.h>
#include <iostream>
#include <stdio.h>
#include <thread>
#include <unistd.h> // sleep()

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
  Launcher(&dynamic, &opts, 12, 13, 14);

  while (1) {
    sleep(2);
  }
  return 0;
}
