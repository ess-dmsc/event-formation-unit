/** Copyright (C) 2016 European Spallation Source */

#include <DGArgs.h>
#include <Socket.h>
#include <Timer.h>
#include <cstring>
#include <inttypes.h>
#include <iostream>
#include <unistd.h>

using namespace std;

typedef std::chrono::high_resolution_clock Clock;

int main(int argc, char *argv[]) {
  DGArgs opts(argc, argv); // Parse command line opts

  char buffer[9000];
  unsigned int seqno = 1;

  const int B1M = 1000000;
  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote("127.0.0.1", opts.port);

  UDPClient DataSource(local, remote);
  DataSource.buflen(opts.buflen);
  DataSource.setbuffers(opts.sndbuf, 0);
  DataSource.printbuffers();

  uint64_t tx_total = 0;
  uint64_t tx = 0;
  uint64_t txp = 0;
  Timer upd;
  auto usecs = upd.timeus();
  for (;;) {
    if ((tx_total + tx) >= (long unsigned int)opts.txGB * 1000000000) {
      cout << "Sent " << tx_total + tx << " bytes." << endl;
      cout << "done" << endl;
      exit(0);
    }
    // Generate Tx buffer
    std::memcpy(buffer, &seqno, sizeof(seqno));
    // Send

    tx += DataSource.send(buffer, opts.buflen);
    if (tx > 0) {
      txp++;
      seqno++;
    } else {
      cout << "unable to send" << endl;
    }

#if 0
    if ((txp % 10000) == 0) {   // Currently this givea approx 10Gbit/s - consider making is an option
      usleep(25000);
    }
#endif

    if (usecs >= opts.updint * 1000000) {
      tx_total += tx;
      printf("Tx rate: %8.2f Mbps, tx %5" PRIu64 " MB (total: %7" PRIu64
             " MB) %ld usecs\n",
             tx * 8.0 / (usecs / 1000000.0) / B1M, tx / B1M, tx_total / B1M,
             usecs);
      tx = 0;

      upd.now();
      usecs = upd.timeus();
    }

    if ((txp % 100) == 0) {
      usecs = upd.timeus();
    }
  }
}
