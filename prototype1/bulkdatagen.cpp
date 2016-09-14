#include <Args.h>
#include <NMXEvent.h>
#include <Socket.h>
#include <chrono>
#include <inttypes.h>
#include <iostream>
#include <unistd.h>

using namespace std;

typedef std::chrono::high_resolution_clock Clock;

int main(int argc, char *argv[]) {

  Args opts(argc, argv); // Parse command line opts

  const int intervalUs = 1000000;
  const int B1M = 1000000;
  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote("127.0.0.1", opts.port);

  VMMBulkData bd;
  UDPClient DataSource(local, remote);
  DataSource.Buflen(opts.buflen);

  uint64_t tx_total = 0;
  uint64_t tx = 0;
  auto t1 = Clock::now();
  for (;;) {

    if (tx_total >= opts.txGB * 1000000000) {
      cout << "Sent " << tx_total << " bytes." << endl;
      cout << "done" << endl;
      exit(0);
    }
    // Generate Tx buffer eventually, for now do nothing
    tx += DataSource.Send();
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
