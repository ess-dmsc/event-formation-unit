/** Copyright (C) 2016 European Spallation Source */

#include <DGArgs.h>
#include <Socket.h>
#include <Timer.h>
#include <cstring>
#include <gccintel.h>
#include <inttypes.h>
#include <iostream>
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

  uint64_t tx_total = 0;
  uint64_t tx = 0;
  uint64_t txp = 0;
  uint64_t tsc0 = rdtsc();
  uint64_t tsc1 = rdtsc();
  uint64_t tsc;
  Timer upd;
  for (;;) {
    tsc = rdtsc();
    if (unlikely((tx_total + tx) >=
                 (long unsigned int)opts.txGB * 1000000000)) {
      cout << "Sent " << tx_total + tx << " bytes." << endl;
      cout << "done" << endl;
      exit(0);
    }
    // Generate Tx buffer
    std::memcpy(buffer, &seqno, sizeof(seqno));

    // Sleep to throttle down speed
    if (unlikely((tsc - tsc1) >= TSC_MHZ * 10000)) {
      usleep(opts.speed_level * 1000);
      tsc1 = rdtsc();
    }

    // Send data
    tx += DataSource.send(buffer, opts.buflen);
    if (tx > 0) {
      txp++;
      seqno++;
    } else {
      cout << "unable to send" << endl;
    }

    if (unlikely(((tsc - tsc0) / TSC_MHZ) >= opts.updint * 1000000)) {
      auto usecs = upd.timeus();
      tx_total += tx;
      printf("Tx rate: %8.2f Mbps, tx %5" PRIu64 " MB (total: %7" PRIu64
             " MB) %ld usecs\n",
             tx * 8.0 / usecs, tx / B1M, tx_total / B1M, usecs);
      tx = 0;
      upd.now();
      tsc0 = rdtsc();
    }
  }
}
