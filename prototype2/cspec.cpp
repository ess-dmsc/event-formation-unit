#include "Detector.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>

using namespace std;

const char *classname = "CSPEC Object";

class CSPEC : public Detector {
public:
  void input_thread(void *a) {
    static int ctr = 0;
    while (1) {
      printf("%s - ctr: %d\n", classname, ctr++);
      sleep(1);
    }
  }
  CSPEC() { cout << "    CSPEC created" << endl; };
  ~CSPEC() { cout << "    CSPEC destroyed" << endl; };
};

class CSPECFactory : public DetectorFactory {
public:
  Detector *create() {
    cout << "    making CSPEC" << endl;
    return new CSPEC;
  }
};

CSPECFactory Factory;
