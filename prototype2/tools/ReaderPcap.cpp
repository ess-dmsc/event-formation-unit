/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <algorithm>
#include <arpa/inet.h>
#include <cassert>
#include <cinttypes>
#include <cstring>
#include <tools/ReaderPcap.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <fmt/format.h>

// GCOVR_EXCL_START

// Protocol identifiers
const int ETHERTYPE_ARP = 0x0806;
const int ETHERTYPE_IPV4 = 0x0800;
const int IPPROTO_UDP = 17;
// Header and data location specifications
const int ETHERTYPE_OFFSET = 12;
#define ETHERNET_HEADER_SIZE 14
#define IP_HEADR_OFFSET      14
#define IP_HEADER_SIZE       20
#define UDP_HEADER_OFFSET    34
#define UDP_HEADER_SIZE       8
#define UDP_DATA_OFFSET      42

ReaderPcap::ReaderPcap(std::string filename)
  : FileName(filename) {
  memset(&Stats, 0, sizeof(struct stats_t));
}


ReaderPcap::~ReaderPcap() {
  if (PcapHandle != NULL ) {
    pcap_close(PcapHandle);
  }
}


int ReaderPcap::open() {
  char errbuff[PCAP_ERRBUF_SIZE];
  PcapHandle = pcap_open_offline(FileName.c_str(), errbuff);
  if (PcapHandle == NULL) {
    return -1;
  }
  return 0;
}


int ReaderPcap::validatePacket(struct pcap_pkthdr *header, const unsigned char *data) {
  Stats.PacketsTotal++; /**< total packets in pcap file */
  Stats.BytesTotal += header->len;

  if (header->len != header->caplen) {
    Stats.PacketsTruncated++;
    return 0;
  }

  uint16_t type = ntohs(*(uint16_t *)&data[ETHERTYPE_OFFSET]);
  // printf("packet header len %d, type %x\n", header->len, type);

  if (type == ETHERTYPE_ARP) {
    Stats.EtherTypeArp++;
  } else if (type == ETHERTYPE_IPV4) {
    Stats.EtherTypeIpv4++;
  } else {
    Stats.EtherTypeUnknown++;
  }

  if (type != ETHERTYPE_IPV4) { // must be ipv4
    return 0;
  }

  struct ip *ip = (struct ip *)&data[IP_HEADR_OFFSET];

  // IPv4 header length must be 20, ip version 4, ipproto must be UDP
  if ((ip->ip_hl != 5) or (ip->ip_v != 4) or (ip->ip_p != IPPROTO_UDP)) {
    Stats.IpProtoUnknown++;
    return 0;
  }
  Stats.IpProtoUDP++;

  assert(header->len > ETHERNET_HEADER_SIZE + IP_HEADER_SIZE + UDP_HEADER_SIZE);
  assert(Stats.PacketsTotal == Stats.EtherTypeIpv4 + Stats.EtherTypeArp + Stats.EtherTypeUnknown);
  assert(Stats.EtherTypeIpv4 == Stats.IpProtoUDP + Stats.IpProtoUnknown);

  struct udphdr *udp = (struct udphdr *)&data[UDP_HEADER_OFFSET];
  #ifndef __FAVOR_BSD // Why is __FAVOR_BSD not defined here?
  uint16_t UdpLen = htons(udp->len);
  #else
  uint16_t UdpLen = htons(udp->uh_ulen);
  #endif
  assert(UdpLen >= UDP_HEADER_SIZE);

  #if 0
  printf("UDP Payload, Packet %" PRIu64 ", time: %d:%d seconds, size: %d bytes\n",
       Stats.PacketsTotal, (int)header->ts.tv_sec, (int)header->ts.tv_usec,
       (int)header->len);
  printf("ip src->dest: 0x%08x:%d ->0x%08x:%d\n",
         ntohl(*(uint32_t*)&ip->ip_src), ntohs(udp->uh_sport),
         ntohl(*(uint32_t*)&ip->ip_dst), ntohs(udp->uh_dport));
  #endif

  return UdpLen;
}

int ReaderPcap::read(char *buffer, size_t bufferlen) {
  if (PcapHandle == nullptr) {
    return -1;
  }

  struct pcap_pkthdr *Header;
  const unsigned char *Data;
  int ret = pcap_next_ex(PcapHandle, &Header, &Data);
  if (ret < 0) {
    return -1;
  }

  int UdpDataLength;
  if ((UdpDataLength = validatePacket(Header, Data)) <= 0) {
    return UdpDataLength;
  }

  auto DataLength = std::min((size_t)(UdpDataLength - UDP_HEADER_SIZE), bufferlen);
  std::memcpy(buffer, &Data[UDP_DATA_OFFSET], DataLength);

  return DataLength;
}


int ReaderPcap::getStats() {
  if (PcapHandle == NULL) {
    return -1;
  }

  while (true) {
    int RetVal;
    struct pcap_pkthdr *Header;
    const unsigned char *Data;
    if ((RetVal = pcap_next_ex(PcapHandle, &Header, &Data))  < 0) {
      break;
    }
    validatePacket(Header, Data);
  }
  return 0;
}


void ReaderPcap::printPacket(unsigned char *data, size_t len) {
  for (unsigned int i = 0; i < len; i++) {
    if ((i % 16) == 0 && i != 0) {
      printf("\n");
    }
    printf("%.2x ", data[i]);
  }
  printf("\n");
}


void ReaderPcap::printStats() {
  printf("Total packets        %" PRIu64 "\n", Stats.PacketsTotal);
  printf("Truncated packets    %" PRIu64 "\n", Stats.PacketsTruncated);
  printf("Ethertype IPv4       %" PRIu64 "\n", Stats.EtherTypeIpv4);
  printf("  ipproto UDP        %" PRIu64 "\n", Stats.IpProtoUDP);
  printf("  ipproto other      %" PRIu64 "\n", Stats.IpProtoUnknown);
  printf("Ethertype unknown    %" PRIu64 "\n", Stats.EtherTypeUnknown);
  printf("Ethertype ARP        %" PRIu64 "\n", Stats.EtherTypeArp);
  printf("Total bytes          %" PRIu64 "\n", Stats.BytesTotal);


}
// GCOVR_EXCL_STOP
