#include <NMXEvent.h>
#include <Socket.h>
#include <chrono>
#include <inttypes.h>
#include <iostream>
#include <unistd.h>

using namespace std;

typedef std::chrono::high_resolution_clock Clock;

int main(int argc, char *argv[]) {
  int c;
  int nmxtuples = 10; // nmx data tuples per bulk data packet
  uint64_t txGB = 10; // stop after this amount of Gbytes has been sent
  int port = 9000;    // UDP destination port

  while ((c = getopt(argc, argv, "hn:s:p:")) != -1)
    switch (c) {
    case 'n':
      nmxtuples = atoi(optarg);
      break;

    case 'p':
      port = atoi(optarg);
      break;

    case 's':
      txGB = atol(optarg);
      break;

    case 'h':
    default:
      cout << "Usage: bulkdatagen [OPTIONS]" << endl;
      cout << " -n tuples      number of data tuples in each UDP packet"
           << endl;
      cout << " -s size        size in GB of transmitted data" << endl;
      cout << " -p port        UDP destination port" << endl;
      cout << " -h             help - prints this message" << endl;
      exit(1);
    }

  cout << "Generating " << txGB << " GB of bulk data packets" << endl;
  cout << "containing " << nmxtuples << " nmx data tuples per packet" << endl;
  cout << "for udp port " << port << endl;

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

    if (tx_total >= txGB * 1000000000) {
      cout << "done" << endl;
      exit(0);
    }
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

      tx = 0;
      t1 = Clock::now();
    }
  }
}
