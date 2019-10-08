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
// Header and data location specifications
const int ETHERNET_HEADER_SIZE = 14;
const int IP_HEADER_SIZE = 20;
const int UDP_HEADER_SIZE = 8;

const int IP_HEADR_OFFSET = ETHERNET_HEADER_SIZE;
const int UDP_HEADER_OFFSET = IP_HEADR_OFFSET + IP_HEADER_SIZE;
const int UDP_DATA_OFFSET = UDP_HEADER_OFFSET + UDP_HEADER_SIZE;

ReaderPcap::ReaderPcap(std::string filename)
  : FileName(filename) {
  memset(&Stats, 0, sizeof(stats_t));
}


ReaderPcap::~ReaderPcap() {
  if (PcapHandle != NULL ) {
    pcap_close(PcapHandle);
  }
}


int ReaderPcap::open() {
  char errbuff[PCAP_ERRBUF_SIZE];
  PcapHandle = pcap_open_offline(FileName.c_str(), errbuff);
  if (PcapHandle == nullptr) {
    return -1;
  }

  if (pcap_compile(PcapHandle, &PcapFilter, FilterUdp, 1, PCAP_NETMASK_UNKNOWN) == -1) {
    return -1;
  }

  return 0;
}


int ReaderPcap::validatePacket(pcap_pkthdr *header, const unsigned char *data) {

  Stats.PacketsTotal++; /**< total packets in pcap file */
  Stats.BytesTotal += header->len;

  if (header->len != header->caplen) {
    Stats.PacketsTruncated++;
    return 0;
  }

  if (pcap_offline_filter(&PcapFilter, header, data) == 0) {
    Stats.PacketsNoMatch++;
    return 0;
  }

  Stats.IpProtoUDP++;
  assert(Stats.PacketsTotal == Stats.PacketsTruncated+ Stats.PacketsNoMatch + Stats.IpProtoUDP);
  assert(header->len > ETHERNET_HEADER_SIZE + IP_HEADER_SIZE + UDP_HEADER_SIZE);

  udphdr *udp = (udphdr *)&data[UDP_HEADER_OFFSET];
  #ifndef __FAVOR_BSD // Why is __FAVOR_BSD not defined here?
  uint16_t UdpLen = htons(udp->len);
  #else
  uint16_t UdpLen = htons(udp->uh_ulen);
  #endif
  assert(UdpLen >= UDP_HEADER_SIZE);

  #if 0
  fmt::print("UDP Payload, Packet {}, time: {}:{} seconds, size: {} bytes\n",
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

  pcap_pkthdr *Header;
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
  if (PcapHandle == nullptr) {
    return -1;
  }

  while (true) {
    int RetVal;
    pcap_pkthdr *Header;
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
  fmt::print("Total packets        {}\n", Stats.PacketsTotal);
  fmt::print("Truncated packets    {}\n", Stats.PacketsTruncated);
  fmt::print("  ipproto UDP        {}\n", Stats.IpProtoUDP);
  fmt::print("  other              {}\n", Stats.PacketsNoMatch);
  fmt::print("Total bytes          {}\n", Stats.BytesTotal);
}
// GCOVR_EXCL_STOP
