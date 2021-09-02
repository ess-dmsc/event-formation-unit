// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/EV42Serializer.h>
#include <freia/FreiaInstrument.h>
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
  0x00, 0x00, 0x00, 0x00,  // GEO 0, TDC 0, VMM 0, CH 0

  // Second readout
  0x02, 0x03, 0x14, 0x00,  // Data Header
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticka
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x00,  // GEO 0, TDC 0, VMM 0, CH 0
};


class FreiaInstrumentTest : public TestBase {
public:


protected:
  struct Counters counters;
  FreiaSettings ModuleSettings;
  EV42Serializer * serializer;

  void SetUp() override {
    ModuleSettings.ConfigFile = ConfigFile;
    ModuleSettings.FilePrefix = "deleteme_";
    serializer = new EV42Serializer(115000, "freia");
    counters = {};

  }
  void TearDown() override {}
};

// Test cases below
TEST_F(FreiaInstrumentTest, Constructor) {
  //BaseSettings Unused;
  FreiaInstrument freia(counters, /*Unused,*/ ModuleSettings, serializer);
  freia.setSerializer(serializer);
  ASSERT_EQ(counters.RingErrors, 0);
  ASSERT_EQ(counters.FENErrors, 0);
}

TEST_F(FreiaInstrumentTest, TwoReadouts) {
  //BaseSettings Unused;


  FreiaInstrument freia(counters, /*Unused,*/ ModuleSettings, serializer);
  freia.setSerializer(serializer);

  auto Res = freia.VMMParser.parse((char *)&MappingError[0], MappingError.size());
  ASSERT_EQ(Res, 2);
  ASSERT_EQ(counters.RingErrors, 0);
  ASSERT_EQ(counters.FENErrors, 0);

  ///
  char Buffer[50];
  memset(Buffer, 0, sizeof(Buffer));
  freia.ESSReadoutParser.Packet.HeaderPtr = (ReadoutParser::PacketHeaderV0 *)&Buffer[0];
  /// !!!! Requires a valid header ptr to previously parsed ESS header
  freia.processReadouts();
  ASSERT_EQ(counters.RingErrors, 1);
  ASSERT_EQ(counters.FENErrors, 1);
}

int main(int argc, char **argv) {
  saveBuffer(ConfigFile, (void *)ConfigStr.c_str(), ConfigStr.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  //deleteFile(ConfigFile);
  return RetVal;
}
