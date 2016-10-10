/** Copyright (C) 2016 European Spallation Source */

#include "Detector.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>

using namespace std;

const char *classname = "CSPEC Object";

class CSPEC : public Detector {
public:
  CSPEC(){};

  ~CSPEC(){};

  void input_thread(void *a __attribute__((unused))) {
    static int ctr = 0;
    while (1) {
      printf("%s - ctr: %d\n", classname, ctr++);
      sleep(1);
    }
  }
};

class CSPECFactory : public DetectorFactory {
public:
  Detector *create() { return new CSPEC; }
};

CSPECFactory Factory;
