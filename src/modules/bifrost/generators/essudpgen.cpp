// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief UDP generator for BIFROST from dat files (Greg T. format)
///
/// Uses real data acquired by CAEN digitisers
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <CLI/CLI.hpp>
#include <bifrost/generators/DatReader.h>
#include <common/system/Socket.h>
#include <generators/PacketGenerator.h>

const uint16_t UdpMaxSizeBytes{8800};

struct {
  std::string FileName;
  std::string IpAddress{"127.0.0.1"};
  uint16_t UDPPort{9000};
  uint32_t TxUSleep{1000};
  uint32_t TxMultiplicity{1};
  uint32_t TxPackets{0xFFFFFFFF};
  uint32_t SkipReadouts{0};
  uint32_t TxReadouts{0xFFFFFFFF};
  uint32_t KernelTxBufferSize{1000000};
  bool Verbose{false};
} Config;

CLI::App app{"Raw MIRACLES text files to UDP data generator"};

int main(int argc, char *argv[]) {
  app.add_option("-i, --ip", Config.IpAddress, "Destination IP address");
  app.add_option("-p, --port", Config.UDPPort, "Destination UDP port");
  app.add_option("-a, --packets", Config.TxPackets, "Packets to send");
  app.add_option("-s, --skip", Config.SkipReadouts, "Skip first readouts");
  app.add_option("-m, --multiplicity", Config.TxMultiplicity,
                 "Repeat packet m times");
  app.add_option("-f, --file", Config.FileName, "Raw DREAM (.txt) file");
  app.add_option("-r, --readouts", Config.TxReadouts, "Readouts to send");
  app.add_option("-t, --throttle", Config.TxUSleep, "usleep between packets");
  app.add_flag("-v, --verbose", Config.Verbose, "print additional info");
  CLI11_PARSE(app, argc, argv);

  BifrostDatReader reader(Config.FileName, Config.Verbose);
  struct BifrostDatReader::dat_data_t DatReadout; // read from dat
  struct BifrostDatReader::udp_data_t UdpReadout; // format to udp
  PacketGenerator gen(ESSReadout::Parser::BIFROST,
                      sizeof(struct BifrostDatReader::udp_data_t));

  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote(Config.IpAddress.c_str(), Config.UDPPort);
  UDPTransmitter DataSource(local, remote);
  DataSource.setBufferSizes(Config.KernelTxBufferSize, 0);
  DataSource.printBufferSizes();

  uint64_t SentPackets = 0;
  uint64_t SentReadouts = 0;
  int res;

  while (((res = reader.readReadout(DatReadout)) > 0) and
         (SentPackets < Config.TxPackets) and
         (SentReadouts < Config.TxReadouts)) {

    // FEN is always 0
    // Ring is fiber
    uint8_t Ring = DatReadout.fiber;
    uint8_t FEN = 0;
    memset(&UdpReadout, 0, sizeof(UdpReadout));
    // UdpReadout.timehi = DatReadout.timehi;
    // UdpReadout.timelow = DatReadout.timelow;
    UdpReadout.OM = 0;
    UdpReadout.tube = DatReadout.tube;
    UdpReadout.ampl_a = DatReadout.ampl_a;
    UdpReadout.ampl_b = DatReadout.ampl_b;
    // add the readout data (skip first four bytes of struct)
    if (SentReadouts >= Config.SkipReadouts) {
      gen.addReadout((uint8_t *)&UdpReadout, Ring, FEN);
    }
    SentReadouts++;

    if (gen.getSize() > UdpMaxSizeBytes) {
      for (unsigned int i = 0; i < Config.TxMultiplicity; i++) {
        DataSource.send(gen.getBuffer(), gen.getSize());
        SentPackets++;
        gen.newPacket(PacketGenerator::IncSeqNum);
      }
      if (Config.TxUSleep != 0) {
        usleep(Config.TxUSleep);
      }
      gen.newPacket(PacketGenerator::ClearPacket);
    }
  }

  if (gen.getSize()) {
    DataSource.send(gen.getBuffer(), gen.getSize());
    SentPackets++;
  }

  printf("Sent %" PRIu64 " packets with %" PRIu64 " readouts\n", SentPackets,
         SentReadouts);
  return 0;
}

// GCOVR_EXCL_STOP
