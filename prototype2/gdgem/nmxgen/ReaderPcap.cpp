/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <algorithm>
#include <arpa/inet.h>
#include <cassert>
#include <cinttypes>
#include <cstring>
#include <gdgem/nmxgen/ReaderPcap.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

ReaderPcap::ReaderPcap(std::string filename) {
  char errbuff[PCAP_ERRBUF_SIZE];
  memset(&stats, 0, sizeof(struct stats_t));

  pcap = pcap_open_offline(filename.c_str(), errbuff);
  if (pcap == NULL) {
    printf("cant open file %s\n", filename.c_str());
  }
}

int ReaderPcap::read(char *buffer, size_t bufferlen) {
#define IPHDROFF 14
#define UDPHDROFF 34
#define UDPDATAOFF 42

  if (pcap == NULL)
    return -1;

  struct pcap_pkthdr *header;
  const unsigned char *data;

  int ret = pcap_next_ex(pcap, &header, &data);
  if (ret < 0)
    return -1;

  stats.rx_pkt++; /**< total packets in pcap file */
  stats.rx_bytes += header->len;

  if (header->len != header->caplen) {
    stats.rx_skipped++;
    return 0;
  }

  uint16_t type = ntohs(*(uint16_t *)&data[12]);

  if (type == 0x0806)
    stats.eth_arp++;
  else if (type == 0x0800)
    stats.eth_ipv4++;
  else
    stats.eth_unkn++;

  if (type != 0x0800) {
    return 0;
  }

  struct ip *ip = (struct ip *)&data[IPHDROFF];

  assert(ip->ip_hl == 5); // IPv4 header length 20
  assert(ip->ip_v == 4);

  if (ip->ip_p != 0x11) { // Not UDP
    stats.ip_unkn++;
    return 0;
  }

  stats.ip_udp++;           // IPv4/UDP
  assert(header->len > 42); // Eth + IP + UDP headers
  assert(stats.rx_pkt == stats.eth_ipv4 + stats.eth_arp + stats.eth_unkn);
  assert(stats.eth_ipv4 == stats.ip_udp + stats.ip_unkn);

  struct udphdr *udp = (struct udphdr *)&data[UDPHDROFF];
#ifndef __FAVOR_BSD // Why is __FAVOR_BSD not defined here?
  uint16_t udplen = htons(udp->len);
#else
  uint16_t udplen = htons(udp->uh_ulen);
#endif
  assert(udplen >= 8);

#if 0
  printf("UDP Payload, Packet %" PRIu64 ", time: %d:%d seconds, size: %d bytes\n",
       stats.rx_pkt, (int)header->ts.tv_sec, (int)header->ts.tv_usec,
       (int)header->len);
  printf("ip src->dest: 0x%08x:%d ->0x%08x:%d\n",
         ntohl(*(uint32_t*)&ip->ip_src), ntohs(udp->uh_sport),
         ntohl(*(uint32_t*)&ip->ip_dst), ntohs(udp->uh_dport));
#endif

  auto len = std::min((size_t)(udplen - 8), bufferlen);
  std::memcpy(buffer, &data[UDPDATAOFF], len);

  return len;
}

void ReaderPcap::printpacket(unsigned char *data, size_t len) {
  for (unsigned int i = 0; i < len; i++) {
    if ((i % 16) == 0 && i != 0) {
      printf("\n");
    }
    printf("%.2x ", data[i]);
  }
  printf("\n");
}

void ReaderPcap::printstats() {
  printf("rx packets %" PRIu64 "\n", stats.rx_pkt);
  printf("  unknown  %" PRIu64 "\n", stats.eth_unkn);
  printf("  arp      %" PRIu64 "\n", stats.eth_arp);
  printf("  ipv4     %" PRIu64 "\n", stats.eth_ipv4);
  printf("    udp    %" PRIu64 "\n", stats.ip_udp);
  printf("    other  %" PRIu64 "\n", stats.ip_unkn);
  printf("rx short   %" PRIu64 "\n", stats.rx_skipped);
  printf("rx bytes   %" PRIu64 "\n", stats.rx_bytes);
}
