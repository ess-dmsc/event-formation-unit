/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cassert>
#include <cspecgen/CspecArgs.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>
#include <unistd.h>

// const int TSC_MHZ = 2900;

int main(int argc, char *argv[]) {
  DGArgs opts(argc, argv);

  char buffer[9000];
  const int readoutdatasize = 40;

  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote(opts.dest_ip.c_str(), opts.port);

  UDPClient DataSource(local, remote);
  DataSource.buflen(opts.buflen);
  DataSource.setbuffers(opts.sndbuf, 0);
  DataSource.printbuffers();

  FILE *f = fopen(opts.filename.c_str(), "r");
  if (f == NULL) {
    printf("error: cannot open file \'%s\'\n", opts.filename.c_str());
    return -1;
  }

  int readsize = (opts.buflen / readoutdatasize) * readoutdatasize;
  assert(readsize <= 9000);
  int readsz;

  uint64_t pkt = 0;
  uint64_t bytes = 0;
  TSCTimer throttle_timer;

  while ((pkt < opts.txPkt) && ((readsz = fread(buffer, 1, readsize, f)) > 0)) {
    DataSource.send(buffer, readsz);
    bytes += readsz;
    pkt++;

    usleep(opts.speed_level * 1000);
  }

  printf("Sent: %" PRIu64 " packets\n", pkt);
  printf("Sent: %" PRIu64 " bytes\n", bytes);

  return 0;
}
