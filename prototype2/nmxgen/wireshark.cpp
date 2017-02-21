/*
* How to read a packet capture file.
*/

/*
* Step 1 - Add includes
*/
#include <arpa/inet.h>
#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <iostream>
#include <pcap.h>
#include <string>
#include <string.h>

class ReaderPcap {
public:
  ReaderPcap(std::string filename);
  int read(char *buffer, size_t bufferlen);

  struct stats_t {
    uint64_t rx_pkt;
    uint64_t rx_skipped;
    uint64_t rx_bytes;
    uint64_t eth_ipv4;
    uint64_t eth_arp;
    uint64_t eth_unkn;
    uint64_t ip_udp;
    uint64_t ip_unkn;
  } stats;


private:
  pcap_t *pcap{NULL};
};

ReaderPcap::ReaderPcap(std::string filename) {
  char errbuff[PCAP_ERRBUF_SIZE];
  memset(&stats, 0, sizeof(struct stats_t));

  pcap = pcap_open_offline(filename.c_str(), errbuff);
  if (pcap == NULL) {
    printf("cant open file %s\n", filename.c_str());
  }
}

int ReaderPcap::read(char *buffer, size_t bufferlen) {
  if (pcap == NULL)
    return -1;

  struct pcap_pkthdr *header;
  const unsigned char *data;

  int ret = pcap_next_ex(pcap, &header, &data);
  if (ret < 0)
    return -1;

  stats.rx_pkt++;
  stats.rx_bytes += header->len;

  uint16_t type = ntohs(*(uint16_t *)&data[12]);

  if (type == 0x0806)
    stats.eth_arp++;
  else if (type == 0x0800)
    stats.eth_ipv4++;
  else
    stats.eth_unkn++;

  if (header->len != header->caplen) {
    stats.rx_skipped++;
    return 0;
  }

  if (type == 0x0800) {       // Ethertype for IPv4
    assert(data[14] == 0x45); // IPv4 header length 20
    if (data[23] != 0x11) {   // UDP
      stats.ip_unkn++;
      return 0;
    }

    uint16_t udplen = htons(*(uint16_t *)&data[38]);
    assert(udplen < 1500);
    assert(udplen >= 8);

    printf("Packet %" PRIu64 ", time: %d:%d seconds, size: %d bytes\n",
           stats.rx_pkt, (int)header->ts.tv_sec, (int)header->ts.tv_usec,
           (int)header->len);

    stats.ip_udp++;
    for (u_int i = 0; (i < header->caplen); i++) {
      if ((i % 16) == 0)
        printf("\n");
      printf("%.2x ", data[i]);
    }
    printf("------------\n");
    for (u_int i = 0; (i < udplen - 8); i++) {
      if ((i % 16) == 0)
        printf("\n");
      printf("%.2x ", data[i + 42]);
    }
    assert(header->caplen > 42);

    printf("\n\n");

    assert(stats.rx_pkt == stats.eth_ipv4 + stats.eth_arp + stats.eth_unkn);
    assert(stats.eth_ipv4 == stats.ip_udp + stats.ip_unkn);

    return 5;
  }
}

int main(int argc, char *argv[]) {
  char buffer[9000];
  if (argc != 2) {
    printf("usage: nmxgenpcap pcapfile\n");
    exit(1);
  }
  std::string pcapfile(argv[1]);
  ReaderPcap pcap(pcapfile);

  int ret;
  while ((ret = pcap.read((char *)&buffer, sizeof(buffer))) != -1) {
    printf("read %d bytes", ret);
  }

#if 0


  printf("rx packets %" PRIu64 "\n", c_rx_pkt);
  printf("  ipv4     %" PRIu64 "\n", c_ipv4);
  printf("    udp    %" PRIu64 "\n", c_ipproto_udp);
  printf("    other  %" PRIu64 "\n", c_ipproto_unkn);
  printf("  arp      %" PRIu64 "\n", c_arp);
  printf("  other    %" PRIu64 "\n", c_unkn);
  printf("rx short   %" PRIu64 "\n", c_rx_skipped);
  printf("rx bytes   %" PRIu64 "\n", c_rx_bytes);

#endif

  return 0;
}
