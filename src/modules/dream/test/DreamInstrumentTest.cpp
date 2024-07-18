// Copyright (C) 2021 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include "common/readout/ess/Parser.h"
#include "common/testutils/HeaderFactory.h"
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <dream/DreamInstrument.h>
#include <memory>
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

std::string ConfigFileMagic{"deleteme_dreaminstrumenttestmagic.json"};
std::string ConfigStrMagic = R"(
  {
    "Detector" : "MAGIC",

    "MaxPulseTimeDiffNS" : 16,

    "Config" : [
      { "Ring" :  0, "FEN":  0, "Type": "PADetector"}
    ]
  }
)";

std::string ConfigFileHeimdal{"deleteme_dreaminstrumenttestheimdal.json"};
std::string ConfigStrHeimdal = R"(
  {
    "Detector" : "HEIMDAL",

    "MaxPulseTimeDiffNS" : 16,

    "Config" : [
      { "Ring" :  0, "FEN":  0, "Type": "HeimdalMantle"}
    ]
  }
)";

class DreamInstrumentTest : public TestBase {
protected:
  struct Counters counters;
  BaseSettings Settings;
  std::unique_ptr<TestHeaderFactory> headerFactory;

  void SetUp() override {
    Settings.ConfigFile = ConfigFile;
    counters = {};
    headerFactory = std::make_unique<TestHeaderFactory>();
  }
  void TearDown() override {}
};

/// Test cases below
TEST_F(DreamInstrumentTest, Constructor) {
  DreamInstrument Dream(counters, Settings);
  ASSERT_EQ(Dream.counters.Readouts, 0);
}

TEST_F(DreamInstrumentTest, ConstructorMagic) {
  Settings.ConfigFile = ConfigFileMagic;
  DreamInstrument Dream(counters, Settings);
  ASSERT_EQ(Dream.counters.Readouts, 0);
}

TEST_F(DreamInstrumentTest, ConstructorHeimdal) {
  Settings.ConfigFile = ConfigFileHeimdal;
  DreamInstrument Dream(counters, Settings);
  ASSERT_EQ(Dream.counters.Readouts, 0);
}

TEST_F(DreamInstrumentTest, CalcPixel) {
  DreamInstrument Dream(counters, Settings);
  DataParser::CDTReadout Data{0, 0, 0, 0, 0, 0, 6, 0, 0};
  ASSERT_EQ(Dream.calcPixel(Dream.DreamConfiguration.RMConfig[0][0], Data), 1);
}

TEST_F(DreamInstrumentTest, CalcPixelMagic) {
  Settings.ConfigFile = ConfigFileMagic;
  DreamInstrument Dream(counters, Settings);
  DataParser::CDTReadout Data{0, 0, 0, 0, 0, 0, 0, 0, 0};
  ASSERT_EQ(Dream.calcPixel(Dream.DreamConfiguration.RMConfig[0][0], Data),
            245760 + 1);
}

TEST_F(DreamInstrumentTest, CalcPixelHeimdal) {
  Settings.ConfigFile = ConfigFileHeimdal;
  DreamInstrument Dream(counters, Settings);
  DataParser::CDTReadout Data{0, 0, 0, 0, 0, 0, 0, 0, 0};
  ASSERT_EQ(Dream.calcPixel(Dream.DreamConfiguration.RMConfig[0][0], Data), 1);
}

TEST_F(DreamInstrumentTest, PulseTimeDiffTooLarge) {
  DreamInstrument Dream(counters, Settings);

  ESSReadout::Parser::PacketHeaderV0 headerv0;
  headerv0.PulseLow = 2; // 88MHz ticks
  headerv0.PrevPulseLow = 0;

  Dream.ESSReadoutParser.Packet.HeaderPtr = headerFactory->createHeader(headerv0);

  ASSERT_EQ(Dream.counters.ReadoutStats.ErrorTimeHigh, 0);
  ASSERT_EQ(Dream.counters.ErrorESSHeaders, 0);

  Dream.processReadouts();

  ASSERT_EQ(Dream.counters.ConfigErrors, 0);
  ASSERT_EQ(Dream.counters.ReadoutStats.ErrorTimeHigh, 1);
  ASSERT_EQ(Dream.counters.ErrorESSHeaders, 1);
}

TEST_F(DreamInstrumentTest, ProcessReadoutsMaxRing) {
  DreamInstrument Dream(counters, Settings);
  Dream.ESSReadoutParser.Packet.HeaderPtr = headerFactory->createHeader(Parser::V0);
  Dream.Serializer = new EV44Serializer(115000, "dream");

  // invalid FiberId
  Dream.DreamParser.Result.push_back({12, 0, 0, 0, 0, 0, 6, 0, 0});
  ASSERT_EQ(Dream.counters.RingMappingErrors, 0);
  Dream.processReadouts();
  ASSERT_EQ(Dream.counters.ConfigErrors, 1);
}

TEST_F(DreamInstrumentTest, ProcessReadoutsMaxFEN) {
  DreamInstrument Dream(counters, Settings);
  Dream.ESSReadoutParser.Packet.HeaderPtr = headerFactory->createHeader(Parser::V0); // new HeaderV0;
  Dream.Serializer = new EV44Serializer(115000, "dream");

  // invalid FENId
  Dream.DreamParser.Result.push_back({0, 12, 0, 0, 0, 0, 6, 0, 0});
  ASSERT_EQ(Dream.counters.FENErrors, 0);
  Dream.processReadouts();
  ASSERT_EQ(Dream.counters.ConfigErrors, 0);
  ASSERT_EQ(Dream.counters.FENMappingErrors, 1);
}

TEST_F(DreamInstrumentTest, ProcessReadoutsConfigError) {
  DreamInstrument Dream(counters, Settings);
  Dream.ESSReadoutParser.Packet.HeaderPtr = headerFactory->createHeader(Parser::V0); // new HeaderV0;
  Dream.Serializer = new EV44Serializer(115000, "dream");

  // unconfigured ring,fen combination
  Dream.DreamParser.Result.push_back({2, 2, 0, 0, 0, 0, 6, 0, 0});
  ASSERT_EQ(Dream.counters.ConfigErrors, 0);
  Dream.processReadouts();
  ASSERT_EQ(Dream.counters.ConfigErrors, 1);
  ASSERT_EQ(Dream.counters.RingMappingErrors, 0);
  ASSERT_EQ(Dream.counters.FENMappingErrors, 0);
}

TEST_F(DreamInstrumentTest, ProcessReadoutsGeometryError) {
  DreamInstrument Dream(counters, Settings);
  Dream.ESSReadoutParser.Packet.HeaderPtr = headerFactory->createHeader(Parser::V0); // new HeaderV0;
  Dream.Serializer = new EV44Serializer(115000, "dream");

  // geometry error (no sumo defined)
  Dream.DreamParser.Result.push_back({0, 0, 0, 0, 0, 0, 0, 0, 0});
  Dream.processReadouts();
  ASSERT_EQ(Dream.counters.ConfigErrors, 0);
  ASSERT_EQ(Dream.counters.RingMappingErrors, 0);
  ASSERT_EQ(Dream.counters.FENMappingErrors, 0);
  ASSERT_EQ(Dream.counters.GeometryErrors, 1);
  ASSERT_EQ(Dream.counters.Events, 0);
}

TEST_F(DreamInstrumentTest, ProcessReadoutsGood) {
  DreamInstrument Dream(counters, Settings);
  Dream.DreamConfiguration.RMConfig[0][0].P2.SumoPair = 6;
  Dream.ESSReadoutParser.Packet.HeaderPtr = headerFactory->createHeader(Parser::V0); // new HeaderV0;I
  Dream.Serializer = new EV44Serializer(115000, "dream");

  // finally an event
  Dream.DreamParser.Result.push_back({0, 0, 0, 0, 0, 0, 6, 0, 0});
  ASSERT_EQ(Dream.counters.Events, 0);
  Dream.processReadouts();
  ASSERT_EQ(Dream.counters.ConfigErrors, 0);
  ASSERT_EQ(Dream.counters.RingMappingErrors, 0);
  ASSERT_EQ(Dream.counters.FENMappingErrors, 0);
  ASSERT_EQ(Dream.counters.GeometryErrors, 0);
  ASSERT_EQ(Dream.counters.Events, 1);
}

int main(int argc, char **argv) {
  saveBuffer(ConfigFile, (void *)ConfigStr.c_str(), ConfigStr.size());
  saveBuffer(ConfigFileMagic, (void *)ConfigStrMagic.c_str(),
             ConfigStrMagic.size());
 saveBuffer(ConfigFileHeimdal, (void *)ConfigStrHeimdal.c_str(),
            ConfigStrHeimdal.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(ConfigFile);
  deleteFile(ConfigFileMagic);
  deleteFile(ConfigFileHeimdal);
  return RetVal;
}
