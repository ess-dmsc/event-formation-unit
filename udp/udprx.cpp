/** Copyright (C) 2016, 2017 European Spallation Source ERIC */
#define __STDC_FORMAT_MACROS 1

#include <Args.h>
#include <cassert>
#include <chrono>
#include <inttypes.h>
#include <iostream>
#include <libs/include/Socket.h>
#include <libs/include/Timer.h>
#include <stdio.h>

typedef std::chrono::high_resolution_clock Clock;

int main(int argc, char *argv[]) {
  Args opts(argc, argv);

  static const int BUFFERSIZE = 9000;
  char buffer[BUFFERSIZE];
  uint64_t rx_total = 0;
  uint64_t rx = 0;
  uint64_t rxp = 0;
  const int intervalUs = 1000000;
  const int B1M = 1000000;

  Socket::Endpoint local("0.0.0.0", 9000);
  UDPReceiver NMX(local);
  NMX.setBufferSizes(0, 500000);
  NMX.printBufferSizes();

  Timer upd;
  auto usecs = upd.timeus();

  for (;;) {
    int rdsize = NMX.receive(buffer, BUFFERSIZE);
    assert(rdsize > 0);
    assert(rdsize == opts.buflen);

    if (rdsize > 0) {
      rx += rdsize;
      rxp++;
    }

    if ((rxp % 100) == 0)
      usecs = upd.timeus();

    if (usecs >= intervalUs) {
      rx_total += rx;
      printf("Rx rate: %.2f Mbps, rx %" PRIu64 " MB (total: %" PRIu64
             " MB) %" PRIu64 " usecs\n",
             rx * 8.0 / (usecs / 1000000.0) / B1M, rx / B1M, rx_total / B1M,
             usecs);
      rx = 0;
      upd.now();
      usecs = upd.timeus();
    }
  }
}
