#include <Socket.h>
#include <Timer.h>
#include <chrono>
#include <inttypes.h>
#include <iostream>
#include <stdio.h>

typedef std::chrono::high_resolution_clock Clock;

int main(int argc, char *argv[]) {
  uint64_t rx_total = 0;
  uint64_t rx = 0;
  const int intervalUs = 1000000;
  const int bytes1M = 1000000;

  UDPServer NMX("0.0.0.0", 9000);

  auto t1 = Clock::now();
  for (;;) {
    rx += NMX.Receive();
    auto t2 = Clock::now();
    auto usecs =
        std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    // printf("Received %d bytes in %" PRIu64 " usecs\n", rx, usecs);
    if (usecs > intervalUs) {
      rx_total += rx;
      printf("Rate: %.2f Mbps, rx %" PRIu64 " bytes (total: %" PRIu64
             ") %ld usecs\n",
             rx * 8.0 / (usecs / 1000000.0) / bytes1M, rx, rx_total, usecs);
      rx = 0;
      t1 = Clock::now();
    }
  }
}
