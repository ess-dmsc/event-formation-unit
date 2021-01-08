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

#include <CLI/CLI.hpp>
#include <loki/generators/RawReader.h>
#include <loki/readout/DataParser.h>
#include <readout/ReadoutParser.h>

class LokiPacketGen {
public:

  LokiPacketGen() {
    newPacket();
  }

  void newPacket() {
    memset(buffer, 0, MaxBytes);

    php = (struct ReadoutParser::PacketHeaderV0 *)buffer;
    php->CookieAndType = ReadoutParser::Loki4Amp << 24;
    php->CookieAndType += 0x535345;
    php->SeqNum = SeqNum++;
    Readouts = 0;
  }

  void setLength(uint16_t Length) {php->TotalLength = Length; }

  uint16_t addReadout(struct Loki::DataParser::LokiReadout & rdout)  {
    uint16_t HeaderSize = sizeof(struct ReadoutParser::PacketHeaderV0);
    uint16_t DataSize = (uint16_t)sizeof(struct Loki::DataParser::LokiReadout);

    char * dptr = buffer + HeaderSize + Readouts * DataSize;
    memcpy(dptr, &rdout, DataSize);
    Readouts++;
    return HeaderSize + DataSize * Readouts;
  }

  uint32_t getSeqNum() { return SeqNum; }

private:
  static const int MaxBytes{9000};
  char buffer[MaxBytes];
  struct ReadoutParser::PacketHeaderV0 * php;
  uint32_t SeqNum{0};
  uint16_t Readouts{0};
};


CLI::App app{"Raw LoKI .dat file to UDP data generator"};

int main(int argc, char * argv[]) {
  std::string FileName;

  app.add_option("-f, --file", FileName, "File");
  CLI11_PARSE(app, argc, argv);

  struct Loki::DataParser::LokiReadout Readout;
  LokiReader reader(FileName);
  LokiPacketGen gen;

  reader.skip(4); // Ignore first 4 bytes

  int res;
  while ((res = reader.readReadout(Readout)) > 0) {
    int size = gen.addReadout(Readout);
    if (size > 8800) {
      gen.newPacket();
    }
  }
  return 0;
}
