/* Copyright (C) 2016-2020 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Wrapper class for reading UDP data from wireshark pcap files
/// one packet at a time.
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <cstdint>
#include <pcap.h>
#include <string>

class ReaderPcap {
public:
  /// \brief construct a reader for a specific file
  /// \param filename name of pcap file
  ReaderPcap(std::string FileName);

  /// closes pcap handle
  ~ReaderPcap();

  /// \brief open file for pcap reading, initialise handle
  /// \return 0 for OK, -1 for error
  int open();

  /// \brief read data from a packet into user specified buffer
  /// \param buffer user allocated buffer, must be at least bufferlen bytes
  /// \param bufferlen length in bytes
  /// \return -1 no more data, 0 non UDP, >0 size of UDP payload
  int read(char *Buffer, size_t BufferSize);

  /// \brief update stats counters, use printStats next
  /// \return 0 on OK, -1 on error (failed open())
  int getStats();

  /// \brief prints a summary of the collected stats
  void printStats();

  /// \brief hexdump of packet, for debug
  void printPacket(unsigned char *Data, size_t PayloadLength);

  struct stats_t {
    uint64_t PacketsTotal;
    uint64_t PacketsTruncated;
    uint64_t BytesTotal;
    uint64_t PacketsNoMatch;
    uint64_t IpProtoUDP;
  } Stats;

private:
  const char * FilterUdp = "ip and udp";
  struct bpf_program PcapFilter;

  /// \brief checking consistency and updating stats
  /// \return 0 for non UDP, >0 udp payload length
  int validatePacket(pcap_pkthdr *Header, const unsigned char *Data);

  std::string FileName;
  pcap_t *PcapHandle{nullptr};
};
// GCOVR_EXCL_STOP
