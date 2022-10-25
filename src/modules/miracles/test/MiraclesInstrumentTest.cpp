// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <miracles/MiraclesInstrument.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <string.h>

using namespace Miracles;

std::string ConfigFile{"deleteme_miracles_instr_config.json"};
std::string ConfigStr = R"(
  {
    "Detector" : "Miracles",

    "MaxPulseTimeNS" : 357142855
  }
)";

class MiraclesInstrumentTest : public TestBase {
protected:
  struct Counters counters;
  MiraclesSettings ModuleSettings;
  EV42Serializer *serializer;
  MiraclesInstrument *miracles;
  ESSReadout::Parser::PacketHeaderV0 PacketHeader;

  void SetUp() override {
    // ModuleSettings.ConfigFile = ConfigFile;
    serializer = new EV42Serializer(115000, "miracles");
    counters = {};

    memset(&PacketHeader, 0, sizeof(PacketHeader));

    miracles = new MiraclesInstrument(counters, ModuleSettings);
    miracles->setSerializer(serializer);
    miracles->ESSReadoutParser.Packet.HeaderPtr = &PacketHeader;
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
TEST_F(MiraclesInstrumentTest, Constructor) {
  ASSERT_EQ(miracles->counters.RxPackets, 0);
  ASSERT_EQ(miracles->counters.Readouts, 0);
}

TEST_F(MiraclesInstrumentTest, CalcPixel) {
  ASSERT_EQ(miracles->calcPixel(0, 0, 0, 1), 1);
}

TEST_F(MiraclesInstrumentTest, InvalidRing) {
  miracles->MiraclesParser.Result.push_back(
      {4, 0, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0});
  miracles->processReadouts();
  ASSERT_EQ(counters.RingErrors, 1);
}

std::vector<uint8_t> InvalidTOF{
    0x00, 0x00, 0x18, 0x00, // Data Header - Ring 4, FEN 0, 24 bytes
    0x00, 0x00, 0x00, 0x00, // Time HI 0 s
    0x00, 0x00, 0x00, 0x00, // Time LO 0 tick
    0x04, 0x00, 0x00, 0x00, // Miracles, Tube 0
    0xaa, 0xaa, 0xbb, 0xbb, // AmpA, AmpB
    0x00, 0x00, 0x00, 0x00  //
};

TEST_F(MiraclesInstrumentTest, InvalidTOF) {
  makeHeader(miracles->ESSReadoutParser.Packet, InvalidTOF);
  ASSERT_EQ(miracles->ESSReadoutParser.Packet.Time.Stats.PrevTofNegative, 0);
  auto Res =
      miracles->MiraclesParser.parse(miracles->ESSReadoutParser.Packet.DataPtr,
                                   miracles->ESSReadoutParser.Packet.DataLength);
  ASSERT_EQ(Res, 1);

  miracles->processReadouts();
  ASSERT_EQ(miracles->ESSReadoutParser.Packet.Time.Stats.PrevTofNegative, 1);
  ASSERT_EQ(miracles->ESSReadoutParser.Packet.Time.Stats.TofNegative, 1);
}

std::vector<uint8_t> NullAmps{
    0x00, 0x00, 0x18, 0x00, // Data Header - Ring 4, FEN 0, 24 bytes
    0x00, 0x00, 0x00, 0x00, // Time HI 0 s
    0x03, 0x00, 0x00, 0x00, // Time LO 0 tick - 0x03 (positive tof)
    0x04, 0x00, 0x00, 0x00, // Miracles, Tube 0
    0x00, 0x00, 0x00, 0x00, // AmpA, AmpB - both 0
    0x00, 0x00, 0x00, 0x00  //
};

TEST_F(MiraclesInstrumentTest, InvalidAmpls) {
  makeHeader(miracles->ESSReadoutParser.Packet, NullAmps);
  auto Res =
      miracles->MiraclesParser.parse(miracles->ESSReadoutParser.Packet.DataPtr,
                                   miracles->ESSReadoutParser.Packet.DataLength);
  ASSERT_EQ(Res, 1);

  miracles->processReadouts();
  ASSERT_EQ(miracles->ESSReadoutParser.Packet.Time.Stats.PrevTofNegative, 0);
  ASSERT_EQ(miracles->ESSReadoutParser.Packet.Time.Stats.TofNegative, 0);
  ASSERT_EQ(counters.PixelErrors, 1);
}

int main(int argc, char **argv) {
  saveBuffer(ConfigFile, (void *)ConfigStr.c_str(), ConfigStr.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(ConfigFile);
  return RetVal;
}
