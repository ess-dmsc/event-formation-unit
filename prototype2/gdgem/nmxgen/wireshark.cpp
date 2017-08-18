/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <arpa/inet.h>
#include <cassert>
#include <cinttypes>
#include <gdgem/nmxgen/NMXArgs.h>
#include <gdgem/nmxgen/ReaderPcap.h>
#include <libs/include/Socket.h>
#include <string.h>
#include <string>

int main(int argc, char *argv[]) {
  NMXArgs opts(argc, argv);
  char buffer[9000];

  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote(opts.dest_ip.c_str(), opts.port);

  UDPClient DataSource(local, remote);
  DataSource.buflen(9000);
  DataSource.setbuffers(1000000, 0);
  DataSource.printbuffers();

  std::string pcapfile(opts.filename);
  ReaderPcap pcap(pcapfile);

  int rdsize;
  uint64_t packets = 0;
  while ((rdsize = pcap.read((char *)&buffer, sizeof(buffer))) != -1) {
    if (rdsize == 0)
      continue; // non udp data

    DataSource.send(buffer, rdsize);
    packets++;
    if (packets >= opts.txPkt) {
      printf("Sent %" PRIu64 " packets, exiting...\n", packets);
      pcap.printstats();
      exit(0);
    }
  }
  printf("Sent %" PRIu64 " packets, exiting...\n", packets);
  // pcap.printstats();

  return 0;
}
