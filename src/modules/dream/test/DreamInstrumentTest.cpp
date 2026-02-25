// Copyright (C) 2021 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/Statistics.h>
#include <common/kafka/EV44Serializer.h>
#include <common/readout/ess/Parser.h>
#include <common/testutils/HeaderFactory.h>
#include <common/testutils/EV44SerializerMock.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <dream/DreamInstrument.h>
#include <memory>
#include <string.h>

using namespace Dream;
using namespace testing;
using HeaderV0 = ESSReadout::Parser::PacketHeaderV0;
using MockSerializer = NiceMock<EV44SerializerMock>;

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
  MockSerializer Serializer;
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
  DreamInstrument<DetectorType::DREAM> Dream(Stats, Counters, Settings, Serializer, ESSHeaderParser);
  ASSERT_EQ(Counters.Readouts, 0);
}

TEST_F(DreamInstrumentTest, ConstructorMagic) {
  Settings.ConfigFile = ConfigFileMagic;
  DreamInstrument<DetectorType::MAGIC> Dream(Stats, Counters, Settings, Serializer, ESSHeaderParser);
  ASSERT_EQ(Counters.Readouts, 0);
}

TEST_F(DreamInstrumentTest, ConstructorHeimdal) {
  Settings.ConfigFile = ConfigFileHeimdal;
  DreamInstrument<DetectorType::HEIMDAL> Dream(Stats, Counters, Settings, Serializer, ESSHeaderParser);
  ASSERT_EQ(Counters.Readouts, 0);
}

TEST_F(DreamInstrumentTest, CalcPixel) {
  DreamInstrument<DetectorType::DREAM> Dream(Stats, Counters, Settings, Serializer, ESSHeaderParser);
  DataParser::CDTReadout Data{0, 0, 0, 0, 0, 0, 6, 0, 0};
  ASSERT_EQ(Dream.getGeometry().calcPixel(Data), 1);
}

TEST_F(DreamInstrumentTest, CalcPixelMagic) {
  Settings.ConfigFile = ConfigFileMagic;
  DreamInstrument<DetectorType::MAGIC> Dream(Stats, Counters, Settings, Serializer, ESSHeaderParser);
  DataParser::CDTReadout Data{0, 0, 0, 0, 0, 0, 0, 0, 0};
  ASSERT_EQ(Dream.getGeometry().calcPixel(Data),
            245760 + 1);
}

TEST_F(DreamInstrumentTest, CalcPixelHeimdal) {
  Settings.ConfigFile = ConfigFileHeimdal;
  DreamInstrument<DetectorType::HEIMDAL> Dream(Stats, Counters, Settings, Serializer, ESSHeaderParser);
  DataParser::CDTReadout Data{0, 0, 0, 0, 0, 0, 0, 0, 0};
  ASSERT_EQ(Dream.getGeometry().calcPixel(Data), 1);
}

TEST_F(DreamInstrumentTest, MaxRing) {
  DreamInstrument<DetectorType::DREAM> Dream(Stats, Counters, Settings, Serializer, ESSHeaderParser);
  ESSHeaderParser.Packet.HeaderPtr = headerFactory->createHeader(Parser::V0);
  ESSHeaderParser.Packet.Time.setReference(ESSTime(1, 100000));
  ESSHeaderParser.Packet.Time.setPrevReference(ESSTime(1, 0));

  // invalid FiberId => Ring=6 which is unconfigured => validation fails
  Dream.DreamParser.Result.push_back({12, 0, 0, 1, 100001, 0, 6, 0, 0});

  // addEvent must NOT be called for a validation error
  EXPECT_CALL(Serializer, addEvent(_, _)).Times(0);

  ASSERT_EQ(Dream.getGeometry().getBaseCounters().RingMappingErrors, 0);
  Dream.processReadouts();
  ASSERT_EQ(Dream.getGeometry().getDreamCounters().ConfigErrors, 1);
  ASSERT_EQ(Counters.Events, 0);
}

TEST_F(DreamInstrumentTest, MaxFEN) {
  DreamInstrument<DetectorType::DREAM> Dream(Stats, Counters, Settings, Serializer, ESSHeaderParser);
  ESSHeaderParser.Packet.HeaderPtr =
      headerFactory->createHeader(Parser::V0); // new HeaderV0;

  // invalid FENId
  Dream.DreamParser.Result.push_back({0, 12, 0, 0, 0, 0, 6, 0, 0});
  ASSERT_EQ(Dream.getGeometry().getBaseCounters().FENErrors, 0);
  Dream.processReadouts();
  ASSERT_EQ(Dream.getGeometry().getDreamCounters().ConfigErrors, 0);
  ASSERT_EQ(Dream.getGeometry().getBaseCounters().FENErrors, 1);
}

TEST_F(DreamInstrumentTest, ConfigError) {
  DreamInstrument<DetectorType::DREAM> Dream(Stats, Counters, Settings, Serializer, ESSHeaderParser);
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

TEST_F(DreamInstrumentTest, GeometryError) {
  DreamInstrument<DetectorType::DREAM> Dream(Stats, Counters, Settings, Serializer, ESSHeaderParser);
  ESSHeaderParser.Packet.HeaderPtr =
      headerFactory->createHeader(Parser::V0);
  ESSHeaderParser.Packet.Time.setReference(ESSTime(1, 100000));
  ESSHeaderParser.Packet.Time.setPrevReference(ESSTime(1, 0));

  // geometry error (no sumo defined) - valid time but bad pixel
  Dream.DreamParser.Result.push_back({0, 0, 0, 1, 100001, 0, 0, 0, 0});

  // addEvent must NOT be called for a geometry error
  EXPECT_CALL(Serializer, addEvent(_, _)).Times(0);

  Dream.processReadouts();
  ASSERT_EQ(Dream.getGeometry().getDreamCounters().ConfigErrors, 0);
  ASSERT_EQ(Dream.getGeometry().getBaseCounters().RingMappingErrors, 0);
  ASSERT_EQ(Dream.getGeometry().getBaseCounters().FENErrors, 0);
  ASSERT_EQ(Dream.getGeometry().getBaseCounters().PixelErrors, 1);
  ASSERT_EQ(Counters.Events, 0);
}

TEST_F(DreamInstrumentTest, GoodEvent) {
  DreamInstrument<DetectorType::DREAM> Dream(Stats, Counters, Settings, Serializer, ESSHeaderParser);
  Dream.getConfiguration().RMConfig[0][0].P2.SumoPair = 6;
  ESSHeaderParser.Packet.HeaderPtr =
      headerFactory->createHeader(Parser::V0);
  // Pulse at (1, 100000), event at (1, 100001) => TOF = 1 tick
  ESSHeaderParser.Packet.Time.setReference(ESSTime(1, 100000));
  ESSHeaderParser.Packet.Time.setPrevReference(ESSTime(1, 0));

  Dream.DreamParser.Result.push_back({0, 0, 0, 1, 100001, 0, 6, 0, 0});

  // addEvent must be called exactly once with non-negative ToF and valid pixel
  EXPECT_CALL(Serializer,
              addEvent(Ge(0), Gt(0)))
      .Times(1);

  ASSERT_EQ(Counters.Events, 0);
  Dream.processReadouts();
  ASSERT_EQ(Dream.getGeometry().getDreamCounters().ConfigErrors, 0);
  ASSERT_EQ(Dream.getGeometry().getBaseCounters().RingMappingErrors, 0);
  ASSERT_EQ(Dream.getGeometry().getBaseCounters().FENErrors, 0);
  ASSERT_EQ(Counters.Events, 1);
}

/// \brief Valid readout: verify the exact ToF value passed to addEvent
TEST_F(DreamInstrumentTest, CorrectTofValue) {
  DreamInstrument<DetectorType::DREAM> Dream(
      Stats, Counters, Settings, Serializer, ESSHeaderParser);
  Dream.getConfiguration().RMConfig[0][0].P2.SumoPair = 6;

  ESSHeaderParser.Packet.HeaderPtr =
      headerFactory->createHeader(Parser::V0);
  // Pulse at (1, 100000), event at (1, 100005) => TOF = 5 ticks
  ESSHeaderParser.Packet.Time.setReference(ESSTime(1, 100000));
  ESSHeaderParser.Packet.Time.setPrevReference(ESSTime(1, 0));

  Dream.DreamParser.Result.push_back({0, 0, 0, 1, 100005, 0, 6, 0, 0});

  int32_t expectedTof = static_cast<int32_t>(5 * ESSTime::ESSClockTick);

  EXPECT_CALL(Serializer,
              addEvent(testing::Eq(expectedTof), _))
      .Times(1);

  Dream.processReadouts();
  EXPECT_EQ(Counters.Events, 1);

  // Verify that getTOF successfully used current pulse
  EXPECT_EQ(ESSHeaderParser.Packet.Time.Counters.TofCount, 1); // Current pulse succeeded
  EXPECT_EQ(ESSHeaderParser.Packet.Time.Counters.TofNegative, 0); // Not negative vs current pulse
}

/// \brief Negative ToF relative to current pulse but valid relative to
/// previous pulse: addEvent must still be called (fallback) with non-negative value
TEST_F(DreamInstrumentTest, NegativeTOFFallback) {
  DreamInstrument<DetectorType::DREAM> Dream(
      Stats, Counters, Settings, Serializer, ESSHeaderParser);
  Dream.getConfiguration().RMConfig[0][0].P2.SumoPair = 6;

  ESSHeaderParser.Packet.HeaderPtr =
      headerFactory->createHeader(Parser::V0);
  // Current pulse at (2, 100000), prev pulse at (2, 0)
  // Event at (2, 50000) => negative vs current, but positive vs prev
  ESSHeaderParser.Packet.Time.setReference(ESSTime(2, 100000));
  ESSHeaderParser.Packet.Time.setPrevReference(ESSTime(2, 0));

  Dream.DreamParser.Result.push_back({0, 0, 0, 2, 50000, 0, 6, 0, 0});

  // Should fall back to prev pulse: TOF = 50000 ticks worth of ns
  int32_t expectedTof = static_cast<int32_t>(50000 * ESSTime::ESSClockTick);

  // addEvent MUST be called with a non-negative ToF (fallback succeeded)
  EXPECT_CALL(Serializer,
              addEvent(testing::AllOf(Ge(0),
                                     testing::Eq(expectedTof)),
                       Gt(0)))
      .Times(1);

  Dream.processReadouts();
  EXPECT_EQ(Counters.Events, 1);

  // Verify that getTOF detected negative ToF and fell back to previous pulse
  EXPECT_EQ(ESSHeaderParser.Packet.Time.Counters.TofCount, 0); // Current pulse failed
  EXPECT_EQ(ESSHeaderParser.Packet.Time.Counters.TofNegative, 1); // Was negative vs current pulse
  EXPECT_EQ(ESSHeaderParser.Packet.Time.Counters.PrevTofCount, 1); // Fallback succeeded
  EXPECT_EQ(ESSHeaderParser.Packet.Time.Counters.PrevTofNegative, 0); // Not negative vs prev pulse
}

/// \brief Event time before both current and previous pulse:
/// getTOF returns nullopt, addEvent must NOT be called
TEST_F(DreamInstrumentTest, NegativePrevTOFError) {
  DreamInstrument<DetectorType::DREAM> Dream(
      Stats, Counters, Settings, Serializer, ESSHeaderParser);
  Dream.getConfiguration().RMConfig[0][0].P2.SumoPair = 6;

  ESSHeaderParser.Packet.HeaderPtr =
      headerFactory->createHeader(Parser::V0);
  // Both pulses at second=5, event at second=1 => before both
  ESSHeaderParser.Packet.Time.setReference(ESSTime(5, 100000));
  ESSHeaderParser.Packet.Time.setPrevReference(ESSTime(5, 0));

  Dream.DreamParser.Result.push_back({0, 0, 0, 1, 0, 0, 6, 0, 0});

  EXPECT_CALL(Serializer, addEvent(_, _)).Times(0);

  Dream.processReadouts();
  EXPECT_EQ(Counters.Events, 0);

  // Verify that getTOF detected negative ToF for both pulses
  EXPECT_EQ(ESSHeaderParser.Packet.Time.Counters.TofCount, 0); // Current pulse failed
  EXPECT_EQ(ESSHeaderParser.Packet.Time.Counters.TofNegative, 1); // Was negative vs current pulse
  EXPECT_EQ(ESSHeaderParser.Packet.Time.Counters.PrevTofCount, 0); // Fallback also failed
  EXPECT_EQ(ESSHeaderParser.Packet.Time.Counters.PrevTofNegative, 1); // Was negative vs prev pulse too
}

/// \brief ToF exceeding MaxTOF (>INT32_MAX ns): addEvent must NOT be called
TEST_F(DreamInstrumentTest, HighTOFError) {
  DreamInstrument<DetectorType::DREAM> Dream(
      Stats, Counters, Settings, Serializer, ESSHeaderParser);
  Dream.getConfiguration().RMConfig[0][0].P2.SumoPair = 6;

  ESSHeaderParser.Packet.HeaderPtr =
      headerFactory->createHeader(Parser::V0);
  // Pulse at (0, 0), event at (3, 0) => TOF ~ 3 seconds > INT32_MAX ns (~2.147s)
  ESSHeaderParser.Packet.Time.setReference(ESSTime(0, 0));
  ESSHeaderParser.Packet.Time.setPrevReference(ESSTime(0, 0));

  Dream.DreamParser.Result.push_back({0, 0, 0, 3, 0, 0, 6, 0, 0});

  EXPECT_CALL(Serializer, addEvent(_, _)).Times(0);

  Dream.processReadouts();
  EXPECT_EQ(Counters.Events, 0);

  // Verify that getTOF detected the high ToF error internally
  EXPECT_EQ(ESSHeaderParser.Packet.Time.Counters.TofHigh, 1);
}

/// \brief Mixed batch: one valid, one invalid ToF, one geometry error.
/// addEvent must be called exactly once with non-negative ToF.
TEST_F(DreamInstrumentTest, MixedTofEvents) {
  DreamInstrument<DetectorType::DREAM> Dream(
      Stats, Counters, Settings, Serializer, ESSHeaderParser);
  Dream.getConfiguration().RMConfig[0][0].P2.SumoPair = 6;

  ESSHeaderParser.Packet.HeaderPtr =
      headerFactory->createHeader(Parser::V0);
  ESSHeaderParser.Packet.Time.setReference(ESSTime(1, 100000));
  ESSHeaderParser.Packet.Time.setPrevReference(ESSTime(1, 0));

  // Valid readout: event at (1, 100002) => TOF = 2 ticks
  Dream.DreamParser.Result.push_back({0, 0, 0, 1, 100002, 0, 6, 0, 0});
  // Invalid ToF: event far in the past => nullopt
  Dream.DreamParser.Result.push_back({0, 0, 0, 0, 0, 0, 6, 0, 0});
  // Geometry error: unconfigured ring
  Dream.DreamParser.Result.push_back({12, 0, 0, 1, 100003, 0, 6, 0, 0});

  // addEvent called exactly once, with non-negative tof
  EXPECT_CALL(Serializer,
              addEvent(Ge(0), Gt(0)))
      .Times(1);

  Dream.processReadouts();
  EXPECT_EQ(Counters.Events, 1);
}

/// \brief Regression guard: addEvent must never receive a negative time value.
/// Uses a custom action to capture and validate all addEvent arguments.
TEST_F(DreamInstrumentTest, AddEventNeverCalledWithNegative) {
  DreamInstrument<DetectorType::DREAM> Dream(
      Stats, Counters, Settings, Serializer, ESSHeaderParser);
  Dream.getConfiguration().RMConfig[0][0].P2.SumoPair = 6;

  ESSHeaderParser.Packet.HeaderPtr =
      headerFactory->createHeader(Parser::V0);
  ESSHeaderParser.Packet.Time.setReference(ESSTime(1, 100000));
  ESSHeaderParser.Packet.Time.setPrevReference(ESSTime(1, 0));

  // Several valid readouts with various tick offsets
  Dream.DreamParser.Result.push_back({0, 0, 0, 1, 100001, 0, 6, 0, 0});
  Dream.DreamParser.Result.push_back({0, 0, 0, 1, 100010, 0, 6, 0, 0});
  Dream.DreamParser.Result.push_back({0, 0, 0, 1, 100100, 0, 6, 0, 0});

  // Capture every addEvent call and assert time >= 0
  std::vector<int32_t> capturedTimes;
  EXPECT_CALL(Serializer, addEvent(_, _))
      .Times(3)
      .WillRepeatedly(testing::DoAll(
          testing::Invoke([&capturedTimes](int32_t time, int32_t) {
            capturedTimes.push_back(time);
          }),
          testing::Return(0)));

  Dream.processReadouts();
  EXPECT_EQ(Counters.Events, 3);

  for (size_t i = 0; i < capturedTimes.size(); i++) {
    EXPECT_GE(capturedTimes[i], 0)
        << "addEvent received negative time at call index " << i
        << ": " << capturedTimes[i];
  }
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
  DreamInstrument<DetectorType::DREAM> Dream(Stats, Counters, Settings, Serializer, ExternalParser);

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
  DreamInstrument<DetectorType::MAGIC> Dream(Stats, Counters, Settings, Serializer, ExternalParser);

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
  DreamInstrument<DetectorType::HEIMDAL> Dream(Stats, Counters, Settings, Serializer, ExternalParser);

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
