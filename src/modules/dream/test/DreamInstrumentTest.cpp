// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <dream/DreamInstrument.h>
#include <string.h>

using namespace Dream;
using HeaderV0 = ESSReadout::Parser::PacketHeaderV0;

std::string ConfigFile{"deleteme_dreaminstrumenttest.json"};
std::string ConfigStr = R"(
  {
    "Detector" : "DREAM",

    "MaxPulseTimeDiffNS" : 16,

    "Config" : [
      { "Ring" :  0, "FEN":  0, "Type": "FwEndCap"}
    ]
  }
)";

class DreamInstrumentTest : public TestBase {
protected:
  struct Counters counters;
  BaseSettings Settings;
  char Header[sizeof(HeaderV0)];

  void SetUp() override {
    memset(Header, 0, sizeof(Header));
    Settings.ConfigFile = ConfigFile;
    counters = {};
  }
  void TearDown() override {}
};

/// Test cases below
TEST_F(DreamInstrumentTest, Constructor) {
  DreamInstrument Dream(counters, Settings);
  ASSERT_EQ(Dream.counters.RxPackets, 0);
  ASSERT_EQ(Dream.counters.Readouts, 0);
}

TEST_F(DreamInstrumentTest, CalcPixel) {
  DreamInstrument Dream(counters, Settings);
  DataParser::DreamReadout Data{0, 0, 0, 0, 0, 0, 0, 0, 0};
  Dream.DreamConfiguration.RMConfig[0][0].P2.SumoPair = 6;
  ASSERT_EQ(Dream.calcPixel(Dream.DreamConfiguration.RMConfig[0][0], Data), 1);
}

TEST_F(DreamInstrumentTest, PulseTimeDiffTooLarge) {
  DreamInstrument Dream(counters, Settings);

  Dream.ESSReadoutParser.Packet.HeaderPtr = (HeaderV0 *)&Header[0];

  ASSERT_EQ(Dream.counters.ReadoutStats.ErrorTimeHigh, 0);
  ASSERT_EQ(Dream.counters.ErrorESSHeaders, 0);

  Dream.ESSReadoutParser.Packet.HeaderPtr->PulseLow = 2; // 88MHz ticks
  Dream.ESSReadoutParser.Packet.HeaderPtr->PrevPulseLow = 0;
  Dream.processReadouts();

  ASSERT_EQ(Dream.counters.ConfigErrors, 0);
  ASSERT_EQ(Dream.counters.ReadoutStats.ErrorTimeHigh, 1);
  ASSERT_EQ(Dream.counters.ErrorESSHeaders, 1);
}

TEST_F(DreamInstrumentTest, ProcessReadoutsMaxRing) {
  DreamInstrument Dream(counters, Settings);
  Dream.ESSReadoutParser.Packet.HeaderPtr = (HeaderV0 *)&Header[0];
  Dream.Serializer = new EV42Serializer(115000, "dream");

  // invalid RingId
  Dream.DreamParser.Result.push_back({12, 0, 0, 0, 0, 0, 0, 0, 0});
  ASSERT_EQ(Dream.counters.RingErrors, 0);
  Dream.processReadouts();
  ASSERT_EQ(Dream.counters.ConfigErrors, 0);
  ASSERT_EQ(Dream.counters.RingErrors, 1);
}

TEST_F(DreamInstrumentTest, ProcessReadoutsMaxFEN) {
  DreamInstrument Dream(counters, Settings);
  Dream.ESSReadoutParser.Packet.HeaderPtr = (HeaderV0 *)&Header[0];
  Dream.Serializer = new EV42Serializer(115000, "dream");

  // invalid FENId
  Dream.DreamParser.Result.push_back({0, 12, 0, 0, 0, 0, 0, 0, 0});
  ASSERT_EQ(Dream.counters.FENErrors, 0);
  Dream.processReadouts();
  ASSERT_EQ(Dream.counters.ConfigErrors, 0);
  ASSERT_EQ(Dream.counters.FENErrors, 1);
}

TEST_F(DreamInstrumentTest, ProcessReadoutsConfigError) {
  DreamInstrument Dream(counters, Settings);
  Dream.ESSReadoutParser.Packet.HeaderPtr = (HeaderV0 *)&Header[0];
  Dream.Serializer = new EV42Serializer(115000, "dream");

  // unconfigured ring,fen combination
  Dream.DreamParser.Result.push_back({2, 2, 0, 0, 0, 0, 0, 0, 0});
  ASSERT_EQ(Dream.counters.ConfigErrors, 0);
  Dream.processReadouts();
  ASSERT_EQ(Dream.counters.ConfigErrors, 1);
  ASSERT_EQ(Dream.counters.RingErrors, 0);
  ASSERT_EQ(Dream.counters.FENErrors, 0);
}

TEST_F(DreamInstrumentTest, ProcessReadoutsGeometryError) {
  DreamInstrument Dream(counters, Settings);
  Dream.ESSReadoutParser.Packet.HeaderPtr = (HeaderV0 *)&Header[0];
  Dream.Serializer = new EV42Serializer(115000, "dream");

  // geometry error (no sumo defined)
  Dream.DreamParser.Result.push_back({0, 0, 0, 0, 0, 0, 0, 0, 0});
  Dream.processReadouts();
  ASSERT_EQ(Dream.counters.ConfigErrors, 0);
  ASSERT_EQ(Dream.counters.RingErrors, 0);
  ASSERT_EQ(Dream.counters.FENErrors, 0);
  ASSERT_EQ(Dream.counters.GeometryErrors, 1);
  ASSERT_EQ(Dream.counters.Events, 0);
}

TEST_F(DreamInstrumentTest, ProcessReadoutsGood) {
  DreamInstrument Dream(counters, Settings);
  Dream.DreamConfiguration.RMConfig[0][0].P2.SumoPair = 6;
  Dream.ESSReadoutParser.Packet.HeaderPtr = (HeaderV0 *)&Header[0];
  Dream.Serializer = new EV42Serializer(115000, "dream");

  // finally an event
  Dream.DreamParser.Result.push_back({0, 0, 0, 0, 0, 0, 0, 0, 0});
  ASSERT_EQ(Dream.counters.Events, 0);
  Dream.processReadouts();
  ASSERT_EQ(Dream.counters.ConfigErrors, 0);
  ASSERT_EQ(Dream.counters.RingErrors, 0);
  ASSERT_EQ(Dream.counters.FENErrors, 0);
  ASSERT_EQ(Dream.counters.GeometryErrors, 0);
  ASSERT_EQ(Dream.counters.Events, 1);
}

int main(int argc, char **argv) {
  saveBuffer(ConfigFile, (void *)ConfigStr.c_str(), ConfigStr.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(ConfigFile);
  return RetVal;
}
