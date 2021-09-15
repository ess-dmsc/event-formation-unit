// Copyright (C) 2021 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate artificial VMM3 readouts for ESS readout system
///
//===----------------------------------------------------------------------===//

#include <CLI/CLI.hpp>
#include <cinttypes>
#include <common/Socket.h>
#include <readout/vmm3/test/ReadoutGenerator.h>
#include <stdio.h>
// GCOVR_EXCL_START

struct {
  /// readout specific
  uint16_t NRings{2};
  uint8_t Type{72};            // Freia (see readout ICD for other instruments)
  /// udp generator generic
  std::string IpAddress{"127.0.0.1"};
  uint16_t UDPPort{9000};
  uint64_t NumberOfPackets{0}; // 0 == all packets
  uint64_t NumReadouts{400};   // # readouts in packet
  uint64_t SpeedThrottle{0};   // 0 is fastest higher is slower
  uint64_t PktThrottle{0};     // 0 is fastest
  bool Loop{false};            // Keep looping the same file forever

  bool Randomise{false};       // Randomise header and data
  // Not yet CLI settings
  uint32_t KernelTxBufferSize{1000000};
} Settings;

CLI::App app{"UDP data generator for ESS VMM3 readout"};

int main(int argc, char *argv[]) {
  app.add_option("-i, --ip", Settings.IpAddress, "Destination IP address");
  app.add_option("-p, --port", Settings.UDPPort, "Destination UDP port");
  app.add_option("-a, --packets", Settings.NumberOfPackets,
                 "Number of packets to send");
  app.add_option("-t, --throttle", Settings.SpeedThrottle,
                 "Speed throttle (0 is fastest, larger is slower)");
  app.add_option("-s, --pkt_throttle", Settings.PktThrottle,
                 "Extra usleep() after n packets");
  app.add_option("-y, --type", Settings.Type,
                 "Detector type id");
  app.add_option("-r, --rings", Settings.NRings,
                 "Number of Rings used in data header");
  app.add_option("-o, --readouts", Settings.NumReadouts,
                "Number of readouts per packet");
  app.add_flag("-m, --random", Settings.Randomise, "Randomise header and data fields");
  app.add_flag("-l, --loop", Settings.Loop, "Run forever");

  CLI11_PARSE(app, argc, argv);

  const int BufferSize{8972};
  uint8_t Buffer[BufferSize];

  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote(Settings.IpAddress.c_str(), Settings.UDPPort);

  UDPTransmitter DataSource(local, remote);
  DataSource.setBufferSizes(Settings.KernelTxBufferSize, 0);
  DataSource.printBufferSizes();

  uint64_t Packets{0};
  uint32_t SeqNum{0};
  ReadoutGenerator gen(Buffer, BufferSize, SeqNum, Settings.Randomise);
  do {
    uint16_t DataSize = gen.makePacket(Settings.Type, Settings.NumReadouts,
      Settings.NRings);

    DataSource.send(Buffer, DataSize);

    if (Settings.SpeedThrottle) {
      usleep(Settings.SpeedThrottle);
    }
    Packets++;
    if (Settings.PktThrottle) {
      if (Packets % Settings.PktThrottle == 0) {
        usleep(10);
      }
    }
    if (Settings.NumberOfPackets != 0 and Packets >= Settings.NumberOfPackets) {
      printf("Sent %" PRIu64 " packets\n", Packets);
      break;
    }
    // printf("Sent %" PRIu64 " packets\n", TotalPackets);
  } while (Settings.Loop or Packets < Settings.NumberOfPackets);
  // pcap.printstats();

  return 0;
}
// GCOVR_EXCL_STOP
