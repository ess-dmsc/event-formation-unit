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
  DreamInstrument Dream(Stats, Counters, Settings, Serializer, ESSHeaderParser);
  ASSERT_EQ(Counters.Readouts, 0);
}

TEST_F(DreamInstrumentTest, ConstructorMagic) {
  Settings.ConfigFile = ConfigFileMagic;
  DreamInstrument Dream(Stats, Counters, Settings, Serializer, ESSHeaderParser);
  ASSERT_EQ(Counters.Readouts, 0);
}

TEST_F(DreamInstrumentTest, ConstructorHeimdal) {
  Settings.ConfigFile = ConfigFileHeimdal;
  DreamInstrument Dream(Stats, Counters, Settings, Serializer, ESSHeaderParser);
  ASSERT_EQ(Counters.Readouts, 0);
}

TEST_F(DreamInstrumentTest, CalcPixel) {
  DreamInstrument Dream(Stats, Counters, Settings, Serializer, ESSHeaderParser);
  DataParser::CDTReadout Data{0, 0, 0, 0, 0, 0, 6, 0, 0};
  ASSERT_EQ(Dream.getGeometry().calcPixel(Data), 1);
}

TEST_F(DreamInstrumentTest, CalcPixelMagic) {
  Settings.ConfigFile = ConfigFileMagic;
  DreamInstrument Dream(Stats, Counters, Settings, Serializer, ESSHeaderParser);
  DataParser::CDTReadout Data{0, 0, 0, 0, 0, 0, 0, 0, 0};
  ASSERT_EQ(Dream.getGeometry().calcPixel(Data),
            245760 + 1);
}

TEST_F(DreamInstrumentTest, CalcPixelHeimdal) {
  Settings.ConfigFile = ConfigFileHeimdal;
  DreamInstrument Dream(Stats, Counters, Settings, Serializer, ESSHeaderParser);
  DataParser::CDTReadout Data{0, 0, 0, 0, 0, 0, 0, 0, 0};
  ASSERT_EQ(Dream.getGeometry().calcPixel(Data), 1);
}

TEST_F(DreamInstrumentTest, ProcessReadoutsMaxRing) {
  DreamInstrument Dream(Stats, Counters, Settings, Serializer, ESSHeaderParser);
  ESSHeaderParser.Packet.HeaderPtr = headerFactory->createHeader(Parser::V0);

  // invalid FiberId
  Dream.DreamParser.Result.push_back({12, 0, 0, 0, 0, 0, 6, 0, 0});
  ASSERT_EQ(Dream.getGeometry().getBaseCounters().RingMappingErrors, 0);
  Dream.processReadouts();
  ASSERT_EQ(Dream.getGeometry().getDreamCounters().ConfigErrors, 1);
}

TEST_F(DreamInstrumentTest, ProcessReadoutsMaxFEN) {
  DreamInstrument Dream(Stats, Counters, Settings, Serializer, ESSHeaderParser);
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
  DreamInstrument Dream(Stats, Counters, Settings, Serializer, ESSHeaderParser);
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
  DreamInstrument Dream(Stats, Counters, Settings, Serializer, ESSHeaderParser);
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
  DreamInstrument Dream(Stats, Counters, Settings, Serializer, ESSHeaderParser);
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

/// AIGEN: Test to verify DreamInstrument creates all counters in external
/// Statistics object passed as reference
TEST_F(DreamInstrumentTest, AIGEN_DreamCounterRegistration) {
  // Create fresh Statistics object to track counter registration
  Statistics Stats("dream", "test");
  size_t InitialCounterCount = Stats.size();

  // Create a new ESSReadout parser with the external stats
  ESSReadout::Parser ExternalParser{Stats};

  // Create DreamInstrument with external Statistics object
  // This should register all counters in Stats
  DreamInstrument Dream(Stats, Counters, Settings, Serializer, ExternalParser);

  // Get the geometry reference
  const Geometry &Geom = Dream.getGeometry();

  // Calculate expected number of counters registered
  // From DreamInstrument and its Geometry:
  // - DetectorGeometry base class registers:
  //   - geometry.ring_errors
  //   - geometry.fen_errors
  //   - geometry.pixel_errors
  // - Dream::Geometry registers:
  //   - geometry.config_errors
  // - ESSReadout::Parser registers its own counters

  size_t FinalCounterCount = Stats.size();
  size_t CountersAddedByDream = FinalCounterCount - InitialCounterCount;

  // Verify that counters were registered
  // We expect at least 4 geometry counters (ring, fen, pixel, config errors)
  // plus potentially parser counters
  EXPECT_GE(CountersAddedByDream, 4)
      << "DreamInstrument should register at least 4 geometry counters "
         "(ring_errors, fen_errors, pixel_errors, config_errors)";

  // Verify specific geometry counters are registered by name
  EXPECT_NE(Stats.getValueByName("geometry.ring_errors"), -1)
      << "Ring mapping errors counter should be registered in external Stats";
  EXPECT_NE(Stats.getValueByName("geometry.fen_errors"), -1)
      << "FEN errors counter should be registered in external Stats";
  EXPECT_NE(Stats.getValueByName("geometry.pixel_errors"), -1)
      << "Pixel errors counter should be registered in external Stats";
  EXPECT_NE(Stats.getValueByName("geometry.config_errors"), -1)
      << "Config errors counter should be registered in external Stats";

  // Verify all geometry counters are initialized to 0
  EXPECT_EQ(Stats.getValueByName("geometry.ring_errors"), 0);
  EXPECT_EQ(Stats.getValueByName("geometry.fen_errors"), 0);
  EXPECT_EQ(Stats.getValueByName("geometry.pixel_errors"), 0);
  EXPECT_EQ(Stats.getValueByName("geometry.config_errors"), 0);

  // Verify base counters through geometry interface
  const auto &BaseCounters = Geom.getBaseCounters();
  EXPECT_EQ(BaseCounters.RingMappingErrors, 0);
  EXPECT_EQ(BaseCounters.FENErrors, 0);
  EXPECT_EQ(BaseCounters.PixelErrors, 0);

  // Verify Dream-specific counters through geometry interface
  const auto &DreamCounters = Geom.getDreamCounters();
  EXPECT_EQ(DreamCounters.ConfigErrors, 0);
}

/// AIGEN: Test DreamInstrument counter registration for Magic variant
TEST_F(DreamInstrumentTest, AIGEN_MagicCounterRegistration) {
  // Setup for Magic variant
  Settings.ConfigFile = ConfigFileMagic;

  // Create fresh Statistics object
  Statistics Stats("dream.magic", "test");
  size_t InitialCounterCount = Stats.size();

  // Create parser with external stats
  ESSReadout::Parser ExternalParser{Stats};

  // Create DreamInstrument with Magic configuration
  DreamInstrument Dream(Stats, Counters, Settings, Serializer, ExternalParser);

  size_t FinalCounterCount = Stats.size();
  size_t CountersAdded = FinalCounterCount - InitialCounterCount;

  // Verify counters were registered for Magic variant
  EXPECT_GE(CountersAdded, 4)
      << "Magic variant should also register at least 4 geometry counters";

  // Verify geometry counters exist in external stats
  EXPECT_NE(Stats.getValueByName("geometry.ring_errors"), -1);
  EXPECT_NE(Stats.getValueByName("geometry.fen_errors"), -1);
  EXPECT_NE(Stats.getValueByName("geometry.pixel_errors"), -1);
  EXPECT_NE(Stats.getValueByName("geometry.config_errors"), -1);

  // Verify counters initialized to 0
  EXPECT_EQ(Stats.getValueByName("geometry.ring_errors"), 0);
  EXPECT_EQ(Stats.getValueByName("geometry.fen_errors"), 0);
  EXPECT_EQ(Stats.getValueByName("geometry.pixel_errors"), 0);
  EXPECT_EQ(Stats.getValueByName("geometry.config_errors"), 0);
}

/// AIGEN: Test DreamInstrument counter registration for Heimdal variant
TEST_F(DreamInstrumentTest, AIGEN_HeimdalCounterRegistration) {
  // Setup for Heimdal variant
  Settings.ConfigFile = ConfigFileHeimdal;

  // Create fresh Statistics object
  Statistics Stats("dream.heimdal", "test");
  size_t InitialCounterCount = Stats.size();

  // Create parser with external stats
  ESSReadout::Parser ExternalParser{Stats};

  // Create DreamInstrument with Heimdal configuration
  DreamInstrument Dream(Stats, Counters, Settings, Serializer, ExternalParser);

  size_t FinalCounterCount = Stats.size();
  size_t CountersAdded = FinalCounterCount - InitialCounterCount;

  // Verify counters were registered for Heimdal variant
  EXPECT_GE(CountersAdded, 4)
      << "Heimdal variant should also register at least 4 geometry counters";

  // Verify geometry counters exist in external stats
  EXPECT_NE(Stats.getValueByName("geometry.ring_errors"), -1);
  EXPECT_NE(Stats.getValueByName("geometry.fen_errors"), -1);
  EXPECT_NE(Stats.getValueByName("geometry.pixel_errors"), -1);
  EXPECT_NE(Stats.getValueByName("geometry.config_errors"), -1);

  // Verify counters initialized to 0
  EXPECT_EQ(Stats.getValueByName("geometry.ring_errors"), 0);
  EXPECT_EQ(Stats.getValueByName("geometry.fen_errors"), 0);
  EXPECT_EQ(Stats.getValueByName("geometry.pixel_errors"), 0);
  EXPECT_EQ(Stats.getValueByName("geometry.config_errors"), 0);
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
