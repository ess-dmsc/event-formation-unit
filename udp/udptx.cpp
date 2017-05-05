#include <Args.h>
#include <libs/include/Socket.h>
#include <libs/include/Timer.h>
#include <libs/include/TSCTimer.h>
#include <inttypes.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

#ifndef PRIu64
#define PRIu64 "ull"
#endif

#define TSC_MHZ 3000

int main(int argc, char *argv[]) {

  Args opts(argc, argv);

  uint64_t tx_total = 0;
  uint64_t tx = 0;
  uint64_t txp = 0;
  const int intervalUs = 1000000;
  const int B1M = 1000000;

  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote(opts.dest_ip.c_str(), opts.port);
  UDPClient udptx(local, remote);
  udptx.buflen(opts.buflen);
  udptx.setbuffers(4000000, 4000000);
  udptx.printbuffers();

  Timer rate_timer;
  TSCTimer report_timer;

  uint32_t seqno = 1;
  for (;;) {
    char buffer[10000];
    *((uint32_t*)buffer) = seqno;
    auto txtmp = udptx.send(buffer, opts.buflen);
    seqno++;

    if (txtmp > 0) {
      txp++;
      tx += txtmp;
    }
    // if (txp % 500 == 0) {
    //    usleep(1000);
    // }

    if (report_timer.timetsc() >= 1000000UL * TSC_MHZ) {
      auto usecs = rate_timer.timeus();
      tx_total += tx;
      printf("Tx rate: %.2f Mbps, %.0f pps, tx %" PRIu64 " MB (total: %" PRIu64
             " MB) %llu usecs\n",
             tx * 8.0 / usecs,
             txp * 1000000.0 / usecs,
             tx / B1M,
             tx_total / B1M,
             usecs);
      tx = 0;
      txp = 0;
      rate_timer.now();
      report_timer.now();
    }
  }
}
