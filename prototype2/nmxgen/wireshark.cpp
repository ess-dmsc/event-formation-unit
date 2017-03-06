/*
* How to read a packet capture file.
*/

/*
* Step 1 - Add includes
*/
#include <arpa/inet.h>
#include <cassert>
#include <cinttypes>
#include <libs/include/Socket.h>
#include <nmxgen/NMXArgs.h>
#include <nmxgen/ReaderPcap.h>
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
