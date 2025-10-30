// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/Statistics.h>
#include <common/kafka/EV44Serializer.h>
#include <common/readout/ess/Parser.h>
#include <common/testutils/HeaderFactory.h>
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
  struct Counters Counters;
  Statistics Stats;
  BaseSettings Settings;
  EV44Serializer Serializer{115000, "dream"};
  ESSReadout::Parser ESSHeaderParser{Stats};
  std::unique_ptr<TestHeaderFactory> headerFactory;

  void SetUp() override {
    Settings.ConfigFile = ConfigFile;
    Counters = {};
    headerFactory = std::make_unique<TestHeaderFactory>();
  }
  void TearDown() override {}
};

/// Test cases below
TEST_F(DreamInstrumentTest, Constructor) {
  DreamInstrument Dream(Counters, Settings, Serializer, ESSHeaderParser);
  ASSERT_EQ(Counters.Readouts, 0);
}

TEST_F(DreamInstrumentTest, ConstructorMagic) {
  Settings.ConfigFile = ConfigFileMagic;
  DreamInstrument Dream(Counters, Settings, Serializer, ESSHeaderParser);
  ASSERT_EQ(Counters.Readouts, 0);
}

TEST_F(DreamInstrumentTest, ConstructorHeimdal) {
  Settings.ConfigFile = ConfigFileHeimdal;
  DreamInstrument Dream(Counters, Settings, Serializer, ESSHeaderParser);
  ASSERT_EQ(Counters.Readouts, 0);
}

TEST_F(DreamInstrumentTest, CalcPixel) {
  DreamInstrument Dream(Counters, Settings, Serializer, ESSHeaderParser);
  DataParser::CDTReadout Data{0, 0, 0, 0, 0, 0, 6, 0, 0};
  ASSERT_EQ(Dream.getGeometry().calcPixel<DataParser::CDTReadout>(Data), 1);
}

TEST_F(DreamInstrumentTest, CalcPixelMagic) {
  Settings.ConfigFile = ConfigFileMagic;
  DreamInstrument Dream(Counters, Settings, Serializer, ESSHeaderParser);
  DataParser::CDTReadout Data{0, 0, 0, 0, 0, 0, 0, 0, 0};
  ASSERT_EQ(Dream.getGeometry().calcPixel<DataParser::CDTReadout>(Data),
            245760 + 1);
}

TEST_F(DreamInstrumentTest, CalcPixelHeimdal) {
  Settings.ConfigFile = ConfigFileHeimdal;
  DreamInstrument Dream(Counters, Settings, Serializer, ESSHeaderParser);
  DataParser::CDTReadout Data{0, 0, 0, 0, 0, 0, 0, 0, 0};
  ASSERT_EQ(Dream.getGeometry().calcPixel<DataParser::CDTReadout>(Data), 1);
}

TEST_F(DreamInstrumentTest, ProcessReadoutsMaxRing) {
  DreamInstrument Dream(Counters, Settings, Serializer, ESSHeaderParser);
  ESSHeaderParser.Packet.HeaderPtr = headerFactory->createHeader(Parser::V0);

  // invalid FiberId
  Dream.DreamParser.Result.push_back({12, 0, 0, 0, 0, 0, 6, 0, 0});
  ASSERT_EQ(Dream.getGeometry().getBaseCounters().RingMappingErrors, 0);
  Dream.processReadouts();
  ASSERT_EQ(Dream.getGeometry().getDreamCounters().ConfigErrors, 1);
}

TEST_F(DreamInstrumentTest, ProcessReadoutsMaxFEN) {
  DreamInstrument Dream(Counters, Settings, Serializer, ESSHeaderParser);
  ESSHeaderParser.Packet.HeaderPtr =
      headerFactory->createHeader(Parser::V0); // new HeaderV0;

  // invalid FENId
  Dream.DreamParser.Result.push_back({0, 12, 0, 0, 0, 0, 6, 0, 0});
  ASSERT_EQ(Dream.getGeometry().getBaseCounters().FENErrors, 0);
  Dream.processReadouts();
  ASSERT_EQ(Dream.getGeometry().getDreamCounters().ConfigErrors, 0);
  ASSERT_EQ(Dream.getGeometry().getBaseCounters().FENErrors, 1);
}

TEST_F(DreamInstrumentTest, ProcessReadoutsConfigError) {
  DreamInstrument Dream(Counters, Settings, Serializer, ESSHeaderParser);
  ESSHeaderParser.Packet.HeaderPtr =
      headerFactory->createHeader(Parser::V0); // new HeaderV0;

  // unconfigured ring,fen combination
  Dream.DreamParser.Result.push_back({2, 2, 0, 0, 0, 0, 6, 0, 0});
  ASSERT_EQ(Dream.getGeometry().getDreamCounters().ConfigErrors, 0);
  Dream.processReadouts();
  ASSERT_EQ(Dream.getGeometry().getDreamCounters().ConfigErrors, 1);
  ASSERT_EQ(Dream.getGeometry().getBaseCounters().RingMappingErrors, 0);
  ASSERT_EQ(Dream.getGeometry().getBaseCounters().FENErrors, 0);
  ASSERT_EQ(Dream.getGeometry().getBaseCounters().PixelErrors, 0);
}

TEST_F(DreamInstrumentTest, ProcessReadoutsGeometryError) {
  DreamInstrument Dream(Counters, Settings, Serializer, ESSHeaderParser);
  ESSHeaderParser.Packet.HeaderPtr =
      headerFactory->createHeader(Parser::V0); // new HeaderV0;

  // geometry error (no sumo defined)
  Dream.DreamParser.Result.push_back({0, 0, 0, 0, 0, 0, 0, 0, 0});
  Dream.processReadouts();
  ASSERT_EQ(Dream.getGeometry().getDreamCounters().ConfigErrors, 0);
  ASSERT_EQ(Dream.getGeometry().getBaseCounters().RingMappingErrors, 0);
  ASSERT_EQ(Dream.getGeometry().getBaseCounters().FENErrors, 0);
  ASSERT_EQ(Dream.getGeometry().getBaseCounters().PixelErrors, 1);
  ASSERT_EQ(Counters.Events, 0);
}

TEST_F(DreamInstrumentTest, ProcessReadoutsGood) {
  DreamInstrument Dream(Counters, Settings, Serializer, ESSHeaderParser);
  Dream.getConfiguration().RMConfig[0][0].P2.SumoPair = 6;
  ESSHeaderParser.Packet.HeaderPtr =
      headerFactory->createHeader(Parser::V0); // new HeaderV0;I

  // finally an event
  Dream.DreamParser.Result.push_back({0, 0, 0, 0, 0, 0, 6, 0, 0});
  ASSERT_EQ(Counters.Events, 0);
  Dream.processReadouts();
  ASSERT_EQ(Dream.getGeometry().getDreamCounters().ConfigErrors, 0);
  ASSERT_EQ(Dream.getGeometry().getBaseCounters().RingMappingErrors, 0);
  ASSERT_EQ(Dream.getGeometry().getBaseCounters().FENErrors, 0);
  ASSERT_EQ(Counters.Events, 1);
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
