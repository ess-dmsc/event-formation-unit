// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief UDP generator from simulated DREAM detector data
///
/// Simulation data from Irina Stefanescu
///
//===----------------------------------------------------------------------===//

#include <common/Socket.h>
#include <CLI/CLI.hpp>
#include <jalousie/generators/SimReader.h>
#include <generators/PacketGenerator.h>

// GCOVR_EXCL_START

const uint16_t UdpMaxSizeBytes{8800};

uint8_t buffer[UdpMaxSizeBytes];

struct {
  std::string FileName;
  std::string IpAddress{"127.0.0.1"};
  uint16_t UDPPort{9000};
  uint32_t TxUSleep{1000};
  uint32_t TxMultiplicity{1};
  uint32_t TxPackets{0xFFFFFFFF};
  uint32_t TxReadouts{0xFFFFFFFF};
  uint32_t KernelTxBufferSize{1000000};
} Config;


CLI::App app{"Raw DREAM .txt file to UDP data generator"};


int main(int argc, char * argv[]) {
  app.add_option("-i, --ip", Config.IpAddress, "Destination IP address");
  app.add_option("-p, --port", Config.UDPPort, "Destination UDP port");
  app.add_option("-a, --packets", Config.TxPackets, "Packets to send");
  app.add_option("-m, --multiplicity", Config.TxMultiplicity, "Repeat packet m times");
  app.add_option("-f, --file", Config.FileName, "Raw DREAM (.txt) file");
  app.add_option("-r, --readouts", Config.TxReadouts, "Readouts to send");
  app.add_option("-t, --throttle", Config.TxUSleep, "usleep between packets");
  CLI11_PARSE(app, argc, argv);

  PacketGenerator gen(ReadoutParser::Debug, sizeof(struct DreamSimReader::sim_data_t));
  DreamSimReader reader(Config.FileName);
  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote(Config.IpAddress.c_str(), Config.UDPPort);

  UDPTransmitter DataSource(local, remote);
  DataSource.setBufferSizes(Config.KernelTxBufferSize, 0);
  DataSource.printBufferSizes();

  struct DreamSimReader::sim_data_t Readout;
  uint64_t SentPackets = 0;
  uint64_t SentReadouts = 0;
  int res;

  while (((res = reader.readReadout(Readout)) > 0) and
         (SentPackets < Config.TxPackets) and
         (SentReadouts < Config.TxReadouts)) {

    if (res == 0) {
      continue;
    }

    gen.addReadout(&Readout, 0, 1); // Ring 0, FEN 1
    SentReadouts++;
    if (gen.getSize() > UdpMaxSizeBytes) {
      for (unsigned int i = 0; i < Config.TxMultiplicity; i++) {
        DataSource.send(gen.getBuffer(), gen.getSize());
        SentPackets++;
        gen.nextSeqNo();
      }
      if (Config.TxUSleep != 0) {
        usleep(Config.TxUSleep);
      }
      gen.newPacket();
    }
  }

  if (gen.getSize()) {
    DataSource.send(gen.getBuffer(), gen.getSize());
    SentPackets++;
  }

  printf("Sent %" PRIu64 " packets with %" PRIu64 " readouts\n", SentPackets, SentReadouts);
  return 0;
}

// GCOVR_EXCL_STOP
