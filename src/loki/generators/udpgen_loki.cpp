/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <CLI/CLI.hpp>
#include <arpa/inet.h>
#include <cassert>
#include <cinttypes>
#include <common/Socket.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <loki/test/ReadoutGenerator.h>
// GCOVR_EXCL_START

struct {
  /// Loki specific
  uint16_t NRings{2};
  //uint16_t FENs{1};
  ///
  std::string IpAddress{"127.0.0.1"};
  uint16_t UDPPort{9000};
  uint64_t NumberOfPackets{0}; // 0 == all packets
  uint64_t SpeedThrottle{0}; // 0 is fastest higher is slower
  uint64_t PktThrottle{0}; // 0 is fastest
  bool Loop{false}; // Keep looping the same file forever
  // Not yet CLI settings
  uint32_t KernelTxBufferSize{1000000};
} Settings;

CLI::App app{"Wireshark file to UDP data generator"};

int main(int argc, char *argv[]) {
  app.add_option("-i, --ip", Settings.IpAddress, "Destination IP address");
  app.add_option("-p, --port", Settings.UDPPort, "Destination UDP port");
  app.add_option("-a, --packets", Settings.NumberOfPackets, "Number of packets to send");
  app.add_option("-t, --throttle", Settings.SpeedThrottle, "Speed throttle (0 is fastest, larger is slower)");
  app.add_option("-s, --pkt_throttle", Settings.PktThrottle, "Extra usleep() after n packets");
  app.add_flag("-l, --loop", Settings.Loop, "Run forever");

  app.add_option("-r, --rings", Settings.NRings, "Number of Rings used in data header");
  CLI11_PARSE(app, argc, argv);

  const int BufferSize{8972};
  uint8_t Buffer[BufferSize];
  const uint16_t DataSections{100};
  const uint16_t DataElements{4};

  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote(Settings.IpAddress.c_str(), Settings.UDPPort);

  UDPTransmitter DataSource(local, remote);
  DataSource.setBufferSizes(Settings.KernelTxBufferSize, 0);
  DataSource.printBufferSizes();

  uint64_t Packets = 0;
  uint64_t TotalPackets = 0;
  do {
    uint16_t DataSize = lokiReadoutDataGen(DataSections,DataElements, Settings.NRings, Buffer, BufferSize);

    uint32_t * SeqNum = (uint32_t *)(Buffer + 24);
    *SeqNum = TotalPackets;

    DataSource.send(Buffer, DataSize);

    if (Settings.SpeedThrottle) {
      usleep(Settings.SpeedThrottle);
    }
    Packets++;
    TotalPackets++;
    if (Settings.PktThrottle) {
      if (Packets % Settings.PktThrottle == 0) {
        usleep(10);
      }
    }
    if (Settings.NumberOfPackets != 0 and Packets >= Settings.NumberOfPackets) {
      printf("Sent %" PRIu64 " packets\n", TotalPackets);
      Packets = 0;
      break;
    }
    //printf("Sent %" PRIu64 " packets\n", TotalPackets);
  } while (Settings.Loop or TotalPackets < Settings.NumberOfPackets);
  // pcap.printstats();

  return 0;
}
// GCOVR_EXCL_STOP
