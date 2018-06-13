/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Wrapper class for reading UDP data from wireshark pcap files
 * one packet at a time.
 */
 /// GCOVR_EXCL_START

#pragma once
#include <cstdint>
#include <pcap.h>
#include <string>

class ReaderPcap {
public:
  /** @brief construct a reader for a specific file
   * @param filename name of pcap file
   */
  ReaderPcap(std::string filename);

  ~ReaderPcap();

  /* @brief read data from a packet into user specified buffer
   * @param buffer user allocated buffer, must be at least bufferlen bytes
   * @param bufferlen length in bytes
   */
  int read(char *buffer, size_t bufferlen);

  void printstats();
  void printpacket(unsigned char *data, size_t payloadlen);

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
  pcap_t *pcap{NULL}; /**< pcap handle used for parsing */
};
/// GCOVR_EXCL_STOP
