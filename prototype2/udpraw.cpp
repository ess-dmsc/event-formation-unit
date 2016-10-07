/** Copyright (C) 2016 European Spallation Source */

#include <Detector.h>
#include <EFUArgs.h>
#include <Socket.h>
#include <Timer.h>
#include <cassert>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

using namespace std;
const char *classname = "UDPRaw Object";

class UDPRaw : public Detector {
public:
  void input_thread(void *a);
  UDPRaw() { cout << "    UDPRaw created" << endl; };
  ~UDPRaw() { cout << "    UDPRaw destroyed" << endl; };
};

void UDPRaw::input_thread(void *args) {
  EFUArgs *opts = (EFUArgs *)args;
  uint64_t rx_total = 0;
  uint64_t rx = 0;
  uint64_t rxp = 0;
  const int intervalUs = 1000000;
  const int B1M = 1000000;

  assert(args != NULL);

  Socket::Endpoint local("0.0.0.0", opts->port);
  UDPServer raw(local);
  raw.printbuffers();
  raw.setbuffers(0, opts->rcvbuf);
  raw.printbuffers();
  Timer upd;
  auto usecs = upd.timeus();

  for (;;) {
    rx += raw.receive();

    if (rx > 0)
      rxp++;

    if ((rxp % 100) == 0)
      usecs = upd.timeus();

    if (usecs >= intervalUs) {
      rx_total += rx;
      printf("Rx rate: %.2f Mbps, rx %" PRIu64 " MB (total: %" PRIu64
             " MB) %ld usecs\n",
             rx * 8.0 / (usecs / 1000000.0) / B1M, rx / B1M, rx_total / B1M,
             usecs);
      rx = 0;
      upd.now();
      usecs = upd.timeus();
    }
  }
}

/** */
class UDPRawFactory : public DetectorFactory {
public:
  Detector *create() {
    cout << "    making UDPRaw" << endl;
    return new UDPRaw;
  }
};

UDPRawFactory Factory;
