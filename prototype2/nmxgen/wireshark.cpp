/*
* How to read a packet capture file.
*/

/*
* Step 1 - Add includes
*/
#include <arpa/inet.h>
#include <cassert>
#include <cinttypes>
#include <string>
#include <string.h>
#include <nmxgen/ReaderPcap.h>
#include <nmxgen/NMXArgs.h>
#include <libs/include/Socket.h>

int main(int argc, char *argv[]) {
  NMXArgs opts(argc, argv);
  char buffer[9000];

  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote("127.0.0.1",9000);

  UDPClient DataSource(local, remote);
  DataSource.buflen(9000);
  DataSource.setbuffers(1000000, 0);
  DataSource.printbuffers();


  std::string pcapfile(opts.filename);
  ReaderPcap pcap(pcapfile);

  int ret;
  uint64_t packets = 0;
  while ((ret = pcap.read((char *)&buffer, sizeof(buffer))) != -1) {
    if (ret == 0)
       continue; // non udp data
    //pcap.printpacket((unsigned char *)buffer, ret);
    DataSource.send();
    packets++;
    if (packets >= opts.txPkt) {
      printf("Sent %" PRIu64 " packets, exiting...", packets);
      pcap.printstats();
      exit(0);
    }

  }

  pcap.printstats();

  return 0;
}
