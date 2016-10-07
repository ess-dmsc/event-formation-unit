#include "Detector.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>

using namespace std;

const char *classname = "NMX Object";

class NMX : public Detector {
public:
  void input_thread(void *a) {
    static int ctr = 0;
    while (1) {
      printf("%s - ctr: %d\n", classname, ctr++);
      sleep(1);
    }
  }
  NMX() { cout << "    NMX created" << endl; };
  ~NMX() { cout << "    NMX destroyed" << endl; };
  virtual void test() { cout << "    NMX tested" << endl; };
};

class NMXFactory : DetectorFactory {
public:
  NMX *create() {
    cout << "    making NMX" << endl;
    return new NMX;
  }
};

NMXFactory Factory;
