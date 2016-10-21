#include <Args.h>
#include <Socket.h>
#include <Timer.h>
#include <chrono>
#include <inttypes.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

#ifndef PRIu64
#define PRIu64 "ull"
#endif

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
  udptx.printbuffers();
  udptx.setbuffers(0, 1000000);
  udptx.printbuffers();

  Timer upd;
  auto usecs = upd.timeus();

  for (;;) {
    tx += udptx.send();

    if (tx > 0)
      txp++;

    if ((txp % 100) == 0)
      usecs = upd.timeus();

    if (usecs >= intervalUs) {
      tx_total += tx;
      printf("Tx rate: %.2f Mbps, tx %" PRIu64 " MB (total: %" PRIu64
             " MB) %ld usecs\n",
             tx * 8.0 / (usecs / 1000000.0) / B1M, tx / B1M, tx_total / B1M,
             usecs);
      tx = 0;
      upd.now();
      usecs = upd.timeus();
    }
  }
}
