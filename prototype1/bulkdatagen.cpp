#include <NMXEvent.h>
#include <Socket.h>
#include <Timer.h>
#include <chrono>
#include <inttypes.h>
#include <iostream>
#include <unistd.h>

using namespace std;

typedef std::chrono::high_resolution_clock Clock;

int main(int argc, char *argv[]) {
  int c;
  int nmxtuples = 10; // nmx data tuples per bulk data packet
  uint64_t bulkdatasz =
      10000000000; // stop after this amount of bytes has been sent

  while ((c = getopt(argc, argv, "n:s:")) != -1)
    switch (c) {
    case 'n':
      nmxtuples = atoi(optarg);
      break;

    case 's':
      bulkdatasz = atol(optarg);
      break;

    default:
      cout << "getopt error" << endl;
      exit(1);
    }

  cout << "Generating " << bulkdatasz << " bytes of bulk data packets" << endl;
  cout << "containing " << nmxtuples << " nmx data tuples per packet" << endl;

  uint64_t tx_total = 0;
  uint64_t tx = 0;
  const int intervalUs = 1000000;
  const int B1M = 1000000;
  struct Endpoint local("0.0.0.0", 0);
  struct Endpoint remote("127.0.0.1", 9000);

  VMMBulkData bd;
  UDPClient DataSource(local, remote);

  auto t1 = Clock::now();
  for (;;) {

    // Generate Tx buffer eventuallys
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
      if (tx_total >= bulkdatasz) {
        exit(0);
      }
      tx = 0;
      t1 = Clock::now();
    }
  }
}
