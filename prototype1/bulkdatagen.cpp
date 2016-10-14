/** Copyright (C) 2016 European Spallation Source */

#include <CSPECData.h>
#include <DGArgs.h>
#include <Socket.h>
#include <Timer.h>
#include <cstring>
#include <gccintel.h>
#include <inttypes.h>
#include <iostream>
#include <unistd.h>

using namespace std;

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

  CSPECData cspec;
  int ndata = 100;
  int size = cspec.generate(buffer, 9000, ndata);
  assert(size == ndata * cspec.datasize);

  uint64_t tx_total = 0;
  uint64_t tx = 0;
  uint64_t txp = 0;
  uint64_t tsc0 = rdtsc();
  uint64_t tsc;
  for (;;) {
    tsc = rdtsc();
    if ((tx_total + tx) >= (long unsigned int)opts.txGB * 1000000000) {
      cout << "Sent " << tx_total + tx << " bytes." << endl;
      cout << "done" << endl;
      exit(0);
    }
    // Generate Tx buffer
    //std::memcpy(buffer, &seqno, sizeof(seqno)); // For NMX

    // Send

    tx += DataSource.send(buffer, size);
    if (tx > 0) {
      txp++;
      seqno++;
    } else {
      cout << "unable to send" << endl;
    }

#if 0
    if ((txp % 10000) == 0) {
      usleep(15000);
    }
#endif

    if (((tsc-tsc0)/2400) >= opts.updint * 1000000) {
      tx_total += tx;
      printf("Tx rate: %8.2f Mbps, tx %5" PRIu64 " MB (total: %7" PRIu64
             " MB) %ld usecs\n",
             tx * 8.0 / (((tsc-tsc0)/2400) / 1000000.0) / B1M, tx / B1M, tx_total / B1M,
             ((tsc-tsc0)/2400));
      tx = 0;
      tsc0 = rdtsc();
    }
  }
}
