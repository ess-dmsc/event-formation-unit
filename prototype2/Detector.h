#include <stdio.h>

using namespace std;

class Detector {
public:
  // default constructor
  virtual void input_thread(void *arg) = 0;
  virtual void processing_thread(void *arg) {
    printf("no processing stage\n");
  };
  virtual void output_thread(void *arg) { printf("no output stage\n"); };
  virtual ~Detector(){};
};

class DetectorFactory {
public:
  virtual Detector *create() = 0;
};
