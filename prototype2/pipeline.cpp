/** Copyright (C) 2016 European Spallation Source */

#include <EFUArgs.h>
#include <Launcher.h>
#include <iostream>
#include <thread>
#include <unistd.h> // sleep()

using namespace std;

/**
 * Load detector, launch pipeline threads, then sleep forever
 */
int main(int argc, char *argv[]) {

  EFUArgs opts(argc, argv);

  cout << "Launching EFU as Instrument " << opts.det << endl;

  Loader dynamic(opts.det);
  if (dynamic.detector == NULL) {
    cout << "Detector not loadable, exiting..." << endl;
    exit(1);
  }

  Launcher(&dynamic, &opts, 12, 13, 14);

  while (1) {
    sleep(2);
  }
  return 0;
}
