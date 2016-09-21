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
  const int B1M = 1000000;

  Socket::Endpoint local("0.0.0.0", 9000);
  UDPServer NMX(local);

  auto t1 = Clock::now();
  for (;;) {
    rx += NMX.receive();
    auto t2 = Clock::now();
    auto usecs =
        std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    if (usecs >= intervalUs) {
      rx_total += rx;
      printf("Rx rate: %.2f Mbps, rx %" PRIu64 " MB (total: %" PRIu64
             " MB) %ld usecs\n",
             rx * 8.0 / (usecs / 1000000.0) / B1M, rx / B1M, rx_total / B1M,
             usecs);
      rx = 0;
      t1 = Clock::now();
    }
  }
}
