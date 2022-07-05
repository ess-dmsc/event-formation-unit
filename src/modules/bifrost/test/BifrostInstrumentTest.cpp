// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <bifrost/BifrostInstrument.h>
#include <common/testutils/SaveBuffer.h>
#include <string.h>
#include <common/testutils/TestBase.h>

using namespace Bifrost;

std::string ConfigFile{"deleteme_bifrost_instr_config.json"};
std::string ConfigStr = R"(
  {
    "Detector" : "Bifrost",

    "MaxPulseTimeNS" : 357142855
  }
)";


std::vector<uint8_t> InvalidTOF{
    0x00, 0x00, 0x18, 0x00, // Data Header - Ring 4, FEN 0, 24 bytes
    0x00, 0x00, 0x00, 0x00, // Time HI 0 s
    0x00, 0x00, 0x00, 0x00, // Time LO 0 tick
    0x04, 0x00, 0x00, 0x00, // Bifrost, Tube 0
    0xaa, 0xaa, 0xbb, 0xbb, // AmpA, AmpB
    0x00, 0x00, 0x00, 0x00  //
};

class BifrostInstrumentTest : public TestBase {
protected:
  struct Counters counters;
  BifrostSettings ModuleSettings;
  EV42Serializer *serializer;
  BifrostInstrument *bifrost;
  ESSReadout::Parser::PacketHeaderV0 PacketHeader;

  void SetUp() override {
    //ModuleSettings.ConfigFile = ConfigFile;
    serializer = new EV42Serializer(115000, "bifrost");
    counters = {};

    memset(&PacketHeader, 0, sizeof(PacketHeader));

    bifrost = new BifrostInstrument(counters, ModuleSettings);
    bifrost->setSerializer(serializer);
    bifrost->ESSReadoutParser.Packet.HeaderPtr = &PacketHeader;
  }

  void TearDown() override {}

  void makeHeader(ESSReadout::Parser::PacketDataV0 &Packet,
                  std::vector<uint8_t> &testdata) {
    Packet.HeaderPtr = &PacketHeader;
    Packet.DataPtr = (char *)&testdata[0];
    Packet.DataLength = testdata.size();
    Packet.Time.setReference(0, 2);
    Packet.Time.setPrevReference(0, 1);
  }

};

/// Test cases below
TEST_F(BifrostInstrumentTest, Constructor) {
  ASSERT_EQ(bifrost->counters.RxPackets, 0);
  ASSERT_EQ(bifrost->counters.Readouts, 0);
}

TEST_F(BifrostInstrumentTest, CalcPixel) {
  ASSERT_EQ(bifrost->calcPixel(0, 0, 0, 1), 1);
}

TEST_F(BifrostInstrumentTest, InvalidRing) {
  bifrost->BifrostParser.Result.push_back({1, 0, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0});
  bifrost->processReadouts();
  ASSERT_EQ(counters.RingErrors, 1);
}

TEST_F(BifrostInstrumentTest, InvalidTOF) {
  makeHeader(bifrost->ESSReadoutParser.Packet, InvalidTOF);
  ASSERT_EQ(bifrost->ESSReadoutParser.Packet.Time.Stats.PrevTofNegative, 0);
  auto Res = bifrost->BifrostParser.parse(
     bifrost->ESSReadoutParser.Packet.DataPtr,
     bifrost->ESSReadoutParser.Packet.DataLength);
  ASSERT_EQ(Res, 1);

  bifrost->processReadouts();
  ASSERT_EQ(bifrost->ESSReadoutParser.Packet.Time.Stats.PrevTofNegative, 1);
  ASSERT_EQ(bifrost->ESSReadoutParser.Packet.Time.Stats.TofNegative, 1);
}

int main(int argc, char **argv) {
  saveBuffer(ConfigFile, (void *)ConfigStr.c_str(), ConfigStr.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(ConfigFile);
  return RetVal;
}
