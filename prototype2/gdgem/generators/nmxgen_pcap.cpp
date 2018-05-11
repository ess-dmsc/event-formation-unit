/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <arpa/inet.h>
#include <cassert>
#include <cinttypes>
#include <gdgem/generators/NMXArgs.h>
#include <gdgem/generators/ReaderPcap.h>
#include <libs/include/Socket.h>
#include <string.h>
#include <string>
#include <unistd.h>

int main(int argc, char *argv[]) {
  NMXArgs opts(argc, argv);
  char buffer[10000];

  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote(opts.dest_ip.c_str(), opts.port);

  UDPTransmitter DataSource(local, remote);
  DataSource.setBufferSizes(1000000, 0);
  DataSource.printBufferSizes();

  std::string pcapfile(opts.filename);

  int rdsize;
  uint64_t packets = 0;
  uint64_t totpackets = 0;
  int pcappackets = 0;
  do {
    ReaderPcap pcap(pcapfile);
    while ((rdsize = pcap.read((char *)&buffer, sizeof(buffer))) != -1) {
      if (rdsize == 0) {
        printf("read non udp data - ignoring\n");
        continue; // non udp data
      }
      pcappackets++;

      if (pcappackets >= opts.pcapoffset) {
        DataSource.send(buffer, rdsize);
        if (opts.throttle) {
          usleep(opts.throttle);
        }
        packets++;
        totpackets++;
        if (packets >= opts.txPkt) {
          printf("Sent %" PRIu64 " packets\n", totpackets);
          packets = 0;
          break;
          // printf("Sent %" PRIu64 " packets, exiting...\n", packets);
          // pcap.printstats();
          // exit(0);
        }
      }
    }

  } while (opts.loop);
  // pcap.printstats();
  printf("Sent %" PRIu64 " packets\n", totpackets);
  return 0;
}
