// Copyright (C) 2021 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate artificial VMM3 readouts for ESS readout system
/// \todo in case of Loki we are not using vmm3, so this is no longer
/// a vmm3 thing
//===----------------------------------------------------------------------===//

#include <CLI/CLI.hpp>
#include <cinttypes>
#include <common/system/Socket.h>
#include <modules/cspec/generators/ReadoutGenerator.h>
#include <modules/cspec/generators/LETReadoutGenerator.h>
#include <modules/freia/generators/ReadoutGenerator.h>
#include <modules/loki/generators/ReadoutGenerator.h>
#include <modules/ttlmonitor/generators/ReadoutGenerator.h>
#include <stdio.h>
// GCOVR_EXCL_START

ReadoutGeneratorBase::GeneratorSettings Settings;

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
  app.add_option("-e, --ev_delay", Settings.TicksBtwEvents,
                 "Delay (ticks) between events");
  app.add_option("-d, --rd_delay", Settings.TicksBtwReadouts,
                 "Delay (ticks) between coincident readouts");
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

  #ifdef FREIA_GENERATOR
    Freia::ReadoutGenerator gen(Buffer, BufferSize, SeqNum, Settings);
  #endif

  #ifdef CSPEC_GENERATOR
    Cspec::ReadoutGenerator gen(Buffer, BufferSize, SeqNum, Settings);
    Settings.Type = ESSReadout::Parser::DetectorType::CSPEC;
  #endif

   #ifdef LET_GENERATOR
    Cspec::LETReadoutGenerator gen(Buffer, BufferSize, SeqNum, Settings);
    Settings.Type = ESSReadout::Parser::DetectorType::CSPEC;
  #endif

  #ifdef LOKI_GENERATOR
   Loki::ReadoutGenerator gen(Buffer, BufferSize, SeqNum, Settings);
   gen.setReadoutDataSize(sizeof(Loki::DataParser::LokiReadout));
   Settings.Type = ESSReadout::Parser::DetectorType::Loki4Amp;
  #endif

  #ifdef TTLMON_GENERATOR
   TTLMonitor::ReadoutGenerator gen(Buffer, BufferSize, SeqNum, Settings);
   Settings.Type = ESSReadout::Parser::DetectorType::TTLMonitor;
  #endif

  do {
    uint16_t DataSize = gen.makePacket();

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
