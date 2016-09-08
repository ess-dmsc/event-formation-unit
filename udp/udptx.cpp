#include <Socket.h>
#include <Timer.h>
#include <chrono>
#include <inttypes.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

typedef std::chrono::high_resolution_clock Clock;

int main(int argc, char *argv[]) {
  uint64_t tx_total = 0;
  uint64_t tx = 0;
  const int intervalUs = 1000000;
  const int bytes1M = 1000000;

  UDPClient VMMBulkData("127.0.0.1", 9000);

  auto t1 = Clock::now();
  for (;;) {
    tx += VMMBulkData.Send();
    auto t2 = Clock::now();
    auto usecs =
        std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    if (usecs > intervalUs) {
      tx_total += tx;
      printf("Rate: %.2f Mbps, rx %" PRIu64 " bytes (total: %" PRIu64
             ") %ld usecs\n",
             tx * 8.0 / (usecs / 1000000.0) / bytes1M, tx, tx_total, usecs);
      tx = 0;
      t1 = Clock::now();
    }
  }
}
