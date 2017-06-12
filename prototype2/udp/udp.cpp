/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cassert>
#include <cinttypes>
#include <common/Detector.h>
#include <common/EFUArgs.h>
#include <iostream>
#include <libs/include/Socket.h>
#include <libs/include/Timer.h>
#include <libs/include/TSCTimer.h>
#include <memory>
#include <stdio.h>
#include <unistd.h>

using namespace std;
const char *classname = "UDPRaw Detector";

#define TSC_MHZ 3000

/** ----------------------------------------------------- */

class UDPRaw : public Detector {
public:
  UDPRaw(void *args);

  ~UDPRaw() { cout << "    UDPRaw destroyed" << endl; }

  void input_thread();

private:
  EFUArgs *opts;
};

UDPRaw::UDPRaw(void *args) {
  opts = (EFUArgs *)args;
  cout << "    UDPRaw created" << endl;
}

void UDPRaw::input_thread() {
  uint64_t rx_total = 0;
  uint64_t rx = 0;
  uint64_t rxp = 0;
  const int B1M = 1000000;

  Socket::Endpoint local(opts->ip_addr.c_str(), opts->port);
  UDPServer raw(local);
  raw.setbuffers(4000000, 4000000);
  //raw.settimeout(0, 100000);
  raw.printbuffers();

  Timer rate_timer;
  TSCTimer report_timer;

  uint32_t seqno = 1;
  uint32_t dropped = 0;
  uint32_t timeseq = 0;
  uint32_t first_dropped = 0;
  for (;;) {
    char buffer[10000];
    auto tmprx = raw.receive(buffer, opts->buflen);
    auto tmpseq = *((uint32_t*)buffer);

    if (seqno == tmpseq) {
      seqno++;
    } else  {
      //printf("seqno: %u, tmpseq: %u\n", seqno, tmpseq);
      dropped+= (tmpseq - seqno);
      seqno = tmpseq + 1;
    }

    if (tmprx > 0) {
      rx += tmprx;
      rxp++;
    }

    if (report_timer.timetsc() >= opts->updint * 1000000UL * TSC_MHZ) {
        timeseq++; 
        auto usecs = rate_timer.timeus();
        if (timeseq == 2) {
          first_dropped = dropped;
          printf("Recorded %d dropped frames as baseline\n", first_dropped);
        }
        rx_total += rx;
        printf("Rx rate: %.2f Mbps, %.0f pps rx %" PRIu64 " MB (total: %" PRIu64
               " MB) %" PRIu64 " usecs, seq_err %u, PER %.2e\n",
               rx * 8.0 / usecs,
               rxp * 1000000.0 / usecs,
               rx / B1M, rx_total / B1M,
               usecs,
               dropped - first_dropped,
               1.0 * (dropped - first_dropped) / (seqno - first_dropped));
        rx = 0;
        rxp = 0;
        rate_timer.now();
        report_timer.now();
      }
  }
}

/** ----------------------------------------------------- */

class UDPRawFactory : public DetectorFactory {
public:
  std::shared_ptr<Detector> create(void *args) {
    return std::shared_ptr<Detector>(new UDPRaw(args));
  }
};

UDPRawFactory Factory;
