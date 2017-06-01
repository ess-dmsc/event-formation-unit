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
  const int B1M = 1000000;

  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote(opts.dest_ip.c_str(), opts.port);
  UDPClient udptx(local, remote);
  udptx.buflen(opts.buflen);
  udptx.setbuffers(4000000, 4000000);
  udptx.printbuffers();

  Timer rate_timer, pkt_rate;
  TSCTimer report_timer;
  TSCTimer pkt_rate_timer;

  uint32_t seqno = 1;
  uint32_t txremain = 0;
  printf("TX: %d pps specified\n", opts.txpps);
  for (;;) {
    char buffer[10000];
    if (txremain) {
      *((uint32_t*)buffer) = seqno;
      auto txtmp = udptx.send(buffer, opts.buflen);
      seqno++;

      if (txtmp > 0) {
        txp++;
        tx += txtmp;
        txremain--;
      }
    } else {
       usleep(1);
    }

    if (pkt_rate_timer.timetsc() >= 10000UL * TSC_MHZ) {
      auto usecs = pkt_rate.timeus();
      txremain = usecs * opts.txpps / 1000000;
      pkt_rate_timer.now();
      pkt_rate.now();
    }

    if (report_timer.timetsc() >= 1000000UL * TSC_MHZ) {
      auto usecs = rate_timer.timeus();
      tx_total += tx;
      //printf("Txremain: %u\n", txremain);
      printf("Tx rate: %.2f Mbps, %.0f pps, tx %" PRIu64 " MB (total: %" PRIu64
             " MB) %" PRIu64 " usecs\n",
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
