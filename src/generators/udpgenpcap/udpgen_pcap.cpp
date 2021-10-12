// Copyright (C) 2016-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Reads pcap files (using ReaderPcap), sends UDP data (to EFU)
///
//===----------------------------------------------------------------------===//

#include <CLI/CLI.hpp>
#include <arpa/inet.h>
#include <cassert>
#include <cinttypes>
#include <generators/udpgenpcap/ReaderPcap.h>
#include <common/system/Socket.h>
#include <string.h>
#include <string>
#include <unistd.h>
// GCOVR_EXCL_START

struct {
  std::string FileName{""};
  std::string IpAddress{"127.0.0.1"};
  uint16_t UDPPort{9000};
  uint64_t NumberOfPackets{0}; // 0 == all packets
  uint64_t SpeedThrottle{0}; // 0 is fastest higher is slower
  uint64_t PktThrottle{0}; // 0 is fastest
  bool Read{false};
  bool Loop{false}; // Keep looping the same file forever
  bool Multicast{false};
  // Not yet CLI settings
  uint64_t PcapOffset{0};
  uint32_t KernelTxBufferSize{1000000};
} Settings;

CLI::App app{"Wireshark file to UDP data generator"};

int main(int argc, char *argv[]) {
  app.add_option("-f, --file", Settings.FileName, "Wireshark PCAP file");
  app.add_option("-i, --ip", Settings.IpAddress, "Destination IP address");
  app.add_option("-p, --port", Settings.UDPPort, "Destination UDP port");
  app.add_option("-a, --packets", Settings.NumberOfPackets, "Number of packets to send");
  app.add_option("-t, --throttle", Settings.SpeedThrottle, "Speed throttle (0 is fastest, larger is slower)");
  app.add_option("-s, --pkt_throttle", Settings.PktThrottle, "Extra usleep() after n packets");
  app.add_flag("-r, --read_only", Settings.Read, "Read pcap file and return stats");
  app.add_flag("-l, --loop", Settings.Loop, "Run forever");
  app.add_flag("-m, --multicast", Settings.Multicast, "Allow IP multicast");
  CLI11_PARSE(app, argc, argv);

  bool IsMulticast = Socket::isMulticast(Settings.IpAddress);

  if (IsMulticast and not Settings.Multicast) {
    printf("IP multicast addresses requires -m flag, exiting...\n");
    return -1;
  }

  Socket::Endpoint LocalEndpoint("0.0.0.0", 0);
  Socket::Endpoint RemoteEndpoint(Settings.IpAddress, Settings.UDPPort);
  UDPTransmitter DataSource(LocalEndpoint, RemoteEndpoint);
  DataSource.setBufferSizes(Settings.KernelTxBufferSize, 0);
  DataSource.printBufferSizes();

  if (IsMulticast) {
    printf("IP multicast!\n");
    DataSource.setMulticastTTL();
  }

  std::string PcapFile(Settings.FileName);

  ReaderPcap Pcap(PcapFile);
  if (Pcap.open() < 0) {
    printf("Error opening file: %s\n", PcapFile.c_str());
    return -1;
  }

  // Don't send UDP, just read through pcap file to get stats
  if (Settings.Read) {
    Pcap.getStats();
    Pcap.printStats();
    return 0;
  }

  uint64_t Packets = 0;
  uint64_t TotPackets = 0;
  uint64_t PcapPackets = 0;
  char RxBuffer[10000];
  do {
    int ReadSize;
    while ((ReadSize = Pcap.read((char *)&RxBuffer, sizeof(RxBuffer))) != -1) {
      if (ReadSize == 0) {
        printf("read non udp data - ignoring\n");
        continue; // non udp data
      }
      PcapPackets++;

      if (PcapPackets >= Settings.PcapOffset) {
        DataSource.send(RxBuffer, ReadSize);
        if (Settings.SpeedThrottle) {
          usleep(Settings.SpeedThrottle);
        }
        Packets++;
        TotPackets++;
        if (Settings.PktThrottle) {
          if (TotPackets % Settings.PktThrottle == 0) {
            usleep(10);
          }
        }
        if (Settings.NumberOfPackets != 0 and Packets >= Settings.NumberOfPackets) {
          printf("Sent %" PRIu64 " packets\n", TotPackets);
          Packets = 0;
          break;
        }
      }
    }
    if (Settings.Loop) {
      Pcap.open();
    } else {
      Pcap.printStats();
    }
    printf("Sent %" PRIu64 " packets\n", TotPackets);
  } while (Settings.Loop);
  // pcap.printstats();

  return 0;
}
// GCOVR_EXCL_STOP
