/** Copyright (C) 2016 European Spallation Source */

#include "Detector.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>

using namespace std;

const char *classname = "CSPEC Detector";

/** ----------------------------------------------------- */

class CSPEC : public Detector {
public:
  class Data {
  public:
    int w0amp, w0pos, g0amp, g0pos;
    int w1amp, w1pos, g1amp, g1pos;
  };

  CSPEC(){};

  ~CSPEC(){};

  void input_thread(void *a);
};


void CSPEC::input_thread(void *a __attribute__((unused))) {
  static int ctr = 0;
  while (1) {
    printf("%s - ctr: %d\n", classname, ctr++);
    sleep(1);
  }
}

/** ----------------------------------------------------- */

class CSPECFactory : public DetectorFactory {
public:
  Detector *create() { return new CSPEC; }
};

CSPECFactory Factory;
