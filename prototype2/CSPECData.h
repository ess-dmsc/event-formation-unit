/** Copyright (C) 2016 European Spallation Source */

#include <cinttypes>
#include <queue>

class CSPECData {
public:
  class Data {
  public:
    unsigned int module{0};
    unsigned int d[8];
    unsigned int time{0};
  };


  CSPECData(char * buffer, int size);

  /** Generate simulated data, place in user specified buffer */
  static int generate(char * buffer, int size, int elems);

  Data data;
  uint64_t ierror{0};
  uint64_t idata{0};
  uint64_t ifrag{0};
  std::queue<Data> dataq;
};
