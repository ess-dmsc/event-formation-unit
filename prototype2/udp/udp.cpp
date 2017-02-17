/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cassert>
#include <cinttypes>
#include <common/Detector.h>
#include <common/EFUArgs.h>
#include <iostream>
#include <libs/include/Socket.h>
#include <libs/include/Timer.h>
#include <memory>
#include <stdio.h>
#include <unistd.h>

#define UNUSED __attribute__((unused))

using namespace std;
const char *classname = "UDPRaw Detector";

/** ----------------------------------------------------- */

class UDPRaw : public Detector {
public:
  void input_thread(void *a);
  UDPRaw(void UNUSED * args) { cout << "    UDPRaw created" << endl; };
  ~UDPRaw() { cout << "    UDPRaw destroyed" << endl; };
};

void UDPRaw::input_thread(void *args) {
  EFUArgs *opts = (EFUArgs *)args;
  uint64_t rx_total = 0;
  uint64_t rx = 0;
  uint64_t rxp = 0;
  const int B1M = 1000000;

  assert(args != NULL);

  Socket::Endpoint local(opts->ip_addr.c_str(), opts->port);
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

    if (usecs >= 1000000) {
      rx_total += rx;
      printf("Rx rate: %.2f Mbps, rx %" PRIu64 " MB (total: %" PRIu64
             " MB) %" PRIu64 " usecs\n",
             rx * 8.0 / usecs, rx / B1M, rx_total / B1M, usecs);
      rx = 0;
      upd.now();
      usecs = upd.timeus();
    }
  }
}

/** ----------------------------------------------------- */

class UDPRawFactory : public DetectorFactory {
public:
  std::shared_ptr<Detector> create(void * args) {
    return std::shared_ptr<Detector>(new UDPRaw(args));
  }
};

UDPRawFactory Factory;
