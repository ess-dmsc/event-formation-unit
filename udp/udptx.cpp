#include <Args.h>
#include <Socket.h>
#include <Timer.h>
#include <chrono>
#include <inttypes.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

typedef std::chrono::high_resolution_clock Clock;

int main(int argc, char *argv[]) {

  Args opts(argc, argv);

  uint64_t tx_total = 0;
  uint64_t tx = 0;
  const int intervalUs = 1000000;
  const int B1M = 1000000;

  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote(opts.dest_ip.c_str(), opts.port);
  UDPClient VMMBulkData(local, remote);
  VMMBulkData.buflen(opts.buflen);

  auto t1 = Clock::now();
  for (;;) {
    tx += VMMBulkData.send();
    auto t2 = Clock::now();
    auto usecs =
        std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    if (usecs >= intervalUs) {
      tx_total += tx;
      printf("Tx rate: %.2f Mbps, tx %" PRIu64 " MB (total: %" PRIu64
             " MB) %ld usecs\n",
             tx * 8.0 / (usecs / 1000000.0) / B1M, tx / B1M, tx_total / B1M,
             usecs);
      tx = 0;
      t1 = Clock::now();
    }
  }
}
