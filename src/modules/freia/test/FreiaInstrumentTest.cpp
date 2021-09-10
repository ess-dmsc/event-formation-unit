// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/EV42Serializer.h>
#include <freia/FreiaInstrument.h>
#include <readout/common/ReadoutParser.h>
#include <test/SaveBuffer.h>
#include <test/TestBase.h>
#include <stdio.h>
#include <string.h>

using namespace Freia;

std::string ConfigFile{"deleteme_freia_instr_config.json"};
std::string ConfigStr = R"(
  {
    "Detector": "Freia",

    "WireChOffset" : 16,

    "Config" : [
      { "Ring" :  0, "CassOffset" :  1, "FENs" : 2},
      { "Ring" :  1, "CassOffset" :  5, "FENs" : 2},
      { "Ring" :  2, "CassOffset" :  9, "FENs" : 2},
      { "Ring" :  3, "CassOffset" : 13, "FENs" : 1},
      { "Ring" :  4, "CassOffset" : 15, "FENs" : 1},
      { "Ring" :  5, "CassOffset" : 17, "FENs" : 1},
      { "Ring" :  6, "CassOffset" : 19, "FENs" : 1},
      { "Ring" :  7, "CassOffset" : 21, "FENs" : 1},
      { "Ring" :  8, "CassOffset" : 23, "FENs" : 1},
      { "Ring" :  9, "CassOffset" : 25, "FENs" : 2},
      { "Ring" : 10, "CassOffset" : 29, "FENs" : 2}
    ],

    "MaxPulseTimeNS" : 50000000

  }
)";


//
std::vector<uint8_t> MappingError {
  // First readout
  0x16, 0x01, 0x14, 0x00,  // Data Header - Ring 22!
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x10,  // GEO 0, TDC 0, VMM 0, CH 16

  // Second readout
  0x02, 0x03, 0x14, 0x00,  // Data Header
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticka
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x01, 0x10,  // GEO 0, TDC 0, VMM 1, CH 16
};


std::vector<uint8_t> WireGap {
  // First readout - plane Y - Wires
  0x04, 0x01, 0x14, 0x00,  // Data Header - Ring 4, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x10,  // GEO 0, TDC 0, VMM 0, CH 16

  0x04, 0x01, 0x14, 0x00,  // Data Header - Ring 4
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x12,  // GEO 0, TDC 0, VMM 0, CH 18

  // Second readout - plane X - Strips
  0x05, 0x01, 0x14, 0x00,  // Data Header, Ring 5, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticka
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x01, 0x10,  // GEO 0, TDC 0, VMM 1, CH 16
};

std::vector<uint8_t> StripGap {
  // First readout - plane Y - Wires
  0x04, 0x01, 0x14, 0x00,  // Data Header - Ring 4, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x10,  // GEO 0, TDC 0, VMM 0, CH 16

  // Second readout - plane X - Strips
  0x05, 0x01, 0x14, 0x00,  // Data Header, Ring 5, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticka
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x01, 0x10,  // GEO 0, TDC 0, VMM 1, CH 16

  0x05, 0x01, 0x14, 0x00,  // Data Header, Ring 5, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticka
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x01, 0x12,  // GEO 0, TDC 0, VMM 1, CH 18
};

std::vector<uint8_t> GoodEvent {
  // First readout - plane Y - Wires
  0x04, 0x01, 0x14, 0x00,  // Data Header - Ring 4, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x10,  // GEO 0, TDC 0, VMM 0, CH 16

  // Second readout - plane X - Strips
  0x05, 0x01, 0x14, 0x00,  // Data Header, Ring 5, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticka
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x01, 0x10,  // GEO 0, TDC 0, VMM 1, CH 16
};


class FreiaInstrumentTest : public TestBase {
public:


protected:
  struct Counters counters;
  FreiaSettings ModuleSettings;
  EV42Serializer * serializer;
  FreiaInstrument * freia;
  char Buffer[50];

  void SetUp() override {
    ModuleSettings.ConfigFile = ConfigFile;
    serializer = new EV42Serializer(115000, "freia");
    counters = {};

    memset(Buffer, 0, sizeof(Buffer));

    freia = new FreiaInstrument(counters, ModuleSettings, serializer);
    freia->setSerializer(serializer);
    freia->ESSReadoutParser.Packet.HeaderPtr = (ReadoutParser::PacketHeaderV0 *)&Buffer[0];
  }
  void TearDown() override {}

  void makeHeader(ReadoutParser::PacketDataV0 & Packet, std::vector<uint8_t> & testdata) {
    Packet.HeaderPtr = (ReadoutParser::PacketHeaderV0 *)&Buffer[0];
    Packet.DataPtr = (char *)&testdata[0];
    Packet.DataLength = testdata.size();
    Packet.Time.setReference(0,0);
    Packet.Time.setPrevReference(0,0);
  }
};

// Test cases below
TEST_F(FreiaInstrumentTest, Constructor) {
  ASSERT_EQ(counters.RingErrors, 0);
  ASSERT_EQ(counters.FENErrors, 0);
}


/// THIS IS NOT A TEST, just ensure we also try dumping to hdf5
TEST_F(FreiaInstrumentTest, DumpTofile) {
  ModuleSettings.FilePrefix = "deleteme_freia_";
  FreiaInstrument FreiaDump(counters, ModuleSettings, serializer);
  FreiaDump.setSerializer(serializer);

  makeHeader(FreiaDump.ESSReadoutParser.Packet, GoodEvent);
  auto Res = FreiaDump.VMMParser.parse(FreiaDump.ESSReadoutParser.Packet);
  FreiaDump.processReadouts();

  counters.VMMStats = FreiaDump.VMMParser.Stats;
  ASSERT_EQ(Res, 2);
  ASSERT_EQ(counters.VMMStats.Readouts, 2);
}

TEST_F(FreiaInstrumentTest, TwoReadouts) {
  makeHeader(freia->ESSReadoutParser.Packet, MappingError);
  auto Res = freia->VMMParser.parse(freia->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 2);
  ASSERT_EQ(counters.RingErrors, 0);
  ASSERT_EQ(counters.FENErrors, 0);

  freia->processReadouts();
  ASSERT_EQ(counters.RingErrors, 1);
  ASSERT_EQ(counters.FENErrors, 1);
}

TEST_F(FreiaInstrumentTest, WireGap) {
  makeHeader(freia->ESSReadoutParser.Packet, WireGap);
  auto Res = freia->VMMParser.parse(freia->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 3);

  freia->processReadouts();
  freia->generateEvents();
  ASSERT_EQ(counters.EventsInvalidWireGap, 1);
}

TEST_F(FreiaInstrumentTest, StripGap) {
  makeHeader(freia->ESSReadoutParser.Packet, StripGap);
  auto Res = freia->VMMParser.parse(freia->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 3);

  freia->processReadouts();
  freia->generateEvents();
  ASSERT_EQ(counters.EventsInvalidStripGap, 1);
}

TEST_F(FreiaInstrumentTest, GoodEvent) {
  makeHeader(freia->ESSReadoutParser.Packet, GoodEvent);
  auto Res = freia->VMMParser.parse(freia->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 2);

  freia->processReadouts();
  freia->generateEvents();
  ASSERT_EQ(counters.Events, 1);
}

int main(int argc, char **argv) {
  saveBuffer(ConfigFile, (void *)ConfigStr.c_str(), ConfigStr.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(ConfigFile);
  return RetVal;
}
