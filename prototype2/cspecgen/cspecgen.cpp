/** Copyright (C) 2016 European Spallation Source */

#include <cspec/CSPECData.h>
#include <cspecgen/CspecArgs.h>
#include <cstring>
#include <inttypes.h>
#include <iostream>
#include <libs/include/Socket.h>
#include <libs/include/Timer.h>
#include <libs/include/gccintel.h>
#include <unistd.h>

using namespace std;

const int TSC_MHZ = 2900;

int main(int argc, char *argv[]) {
  DGArgs opts(argc, argv); // Parse command line opts

  char buffer[9000];
  unsigned int seqno = 1;

  const int B1M = 1000000;
  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote(opts.dest_ip.c_str(), opts.port);

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
  uint64_t tsc1 = rdtsc();
  uint64_t tsc;
  for (;;) {
    tsc = rdtsc();
    if (unlikely((tx_total + tx) >= (long unsigned int)opts.txGB * 1000000000)) {
      cout << "Sent " << tx_total + tx << " bytes." << endl;
      cout << "done" << endl;
      exit(0);
    }

    // Sleep to throttle down speed
    if (unlikely((tsc - tsc1) >= TSC_MHZ * 10000)) {
      usleep(opts.speed_level * 1000);
      tsc1 = rdtsc();
    }

    // Send data
    int wrsize = DataSource.send(buffer, size);
    if (wrsize > 0) {
      tx += wrsize;
      txp++;
      seqno++;
    } else {
      cout << "unable to send" << endl;
    }

    if (unlikely(((tsc - tsc0) / TSC_MHZ) >= opts.updint * 1000000)) {
      tx_total += tx;
      printf("Tx rate: %8.2f Mbps, tx %5" PRIu64 " MB (total: %7" PRIu64
             " MB) %ld usecs\n",
             tx * 8.0 / (((tsc - tsc0) / TSC_MHZ) / 1000000.0) / B1M, tx / B1M,
             tx_total / B1M, ((tsc - tsc0) / TSC_MHZ));
      tx = 0;
      tsc0 = rdtsc();
    }
  }
}
