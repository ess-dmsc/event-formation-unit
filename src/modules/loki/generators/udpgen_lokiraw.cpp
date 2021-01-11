// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief UDP generator from raw detector data
///
/// Raw data format gotten from Davide Raspino at STFC
///
//===----------------------------------------------------------------------===//

#include <common/Socket.h>
#include <CLI/CLI.hpp>
#include <loki/generators/RawReader.h>
#include <loki/generators/LokiPacketGen.h>


struct {
  std::string FileName;
  std::string IpAddress{"127.0.0.1"};
  uint16_t UDPPort{9000};
  uint32_t TxUSleep{1000};
  uint32_t TxPackets{0xFFFFFFFF};
  uint32_t TxReadouts{0xFFFFFFFF};
  uint32_t KernelTxBufferSize{1000000};
} Config;


CLI::App app{"Raw LoKI .dat file to UDP data generator"};


int main(int argc, char * argv[]) {
  app.add_option("-a, --packets", Config.TxPackets, "Packets to send");
  app.add_option("-r, --readouts", Config.TxReadouts, "Readouts to send");
  app.add_option("-f, --file", Config.FileName, "Raw LokI (.dat) file");
  app.add_option("-t, --throttle", Config.TxUSleep, "usleep between packets");
  CLI11_PARSE(app, argc, argv);


  LokiReader reader(Config.FileName);
  LokiPacketGen gen;
  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote(Config.IpAddress.c_str(), Config.UDPPort);

  UDPTransmitter DataSource(local, remote);
  DataSource.setBufferSizes(Config.KernelTxBufferSize, 0);
  DataSource.printBufferSizes();

  struct Loki::DataParser::LokiReadout Readout;
  uint64_t SentPackets = 0;
  uint64_t SentReadouts = 0;
  int res, size;
  //reader.skip(4); // Ignore first 4 bytes
  while (((res = reader.readReadout(Readout)) > 0) and
         (SentPackets < Config.TxPackets) and
         (SentReadouts < Config.TxReadouts)) {
    size = gen.addReadout(Readout, 0, 1); // Ring 0, FEN 1
    SentReadouts++;
    if (size > 8800) {
      gen.setLength(size);
      DataSource.send(gen.getBuffer(), size);
      if (Config.TxUSleep != 0) {
        usleep(Config.TxUSleep);
      }
      SentPackets++;
      gen.newPacket();
    }
  }
  gen.setLength(size);
  DataSource.send(gen.getBuffer(), size);
  SentPackets++;
  printf("Sent %llu packets with %llu readouts\n", SentPackets, SentReadouts);
  return 0;
}
