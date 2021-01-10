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
  uint32_t TxPackets{0xFFFFFFFF};
  uint32_t KernelTxBufferSize{1000000};
} Config;


CLI::App app{"Raw LoKI .dat file to UDP data generator"};


int main(int argc, char * argv[]) {

  app.add_option("-a, --packets", Config.TxPackets, "Packets to send");
  app.add_option("-f, --file", Config.FileName, "Raw LokI (.dat) file");
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
  int res;
  //reader.skip(4); // Ignore first 4 bytes
  while (((res = reader.readReadout(Readout)) > 0) and (SentPackets < Config.TxPackets)) {
    int size = gen.addReadout(Readout, 0, 1); // Ring 0, FEN 1
    if (size > 8800) {
      gen.setLength(size);
      DataSource.send(gen.getBuffer(), size);
      SentPackets++;
      gen.newPacket();
    }
  }
  return 0;
}
