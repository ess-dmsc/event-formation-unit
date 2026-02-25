// Copyright (C) 2020 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include "common/time/ESSTime.h"
#include <caen/CaenInstrument.h>
#include <common/Statistics.h>
#include <common/readout/ess/Parser.h>
#include <common/testutils/EV44SerializerMock.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <readout/DataParser.h>

#include <optional>

using namespace Caen;
using namespace testing;

using MockSerializer = NiceMock<EV44SerializerMock>;
using MockSerializerPtr = std::shared_ptr<MockSerializer>;

class CaenInstrumentTest : public TestBase {
protected:
  struct CaenCounters counters;
  Statistics stats;
  ESSReadout::Parser parser{stats};
  BaseSettings Settings;
  DataParser::CaenReadout readout;
  std::vector<std::shared_ptr<EV44Serializer>> serializers;

  void SetUp() override {
    Settings.DetectorName = "loki";
    Settings.ConfigFile = LOKI_CONFIG;

    // Initialize common readout object with default values for Loki
    readout.FiberId = 0; // For Ring 0
    readout.FENId = 0;   // Valid FEN ID for Ring 0
    // This tim high and time low are set to simulate a readout
    // that is just below the default MaxTOF threshold (about 1.4 seconds
    readout.TimeHigh = 1001;
    readout.TimeLow = 37736785;
    readout.AmpA = 20; // Increased for better position calculation
    readout.AmpB = 20; // Ensure B+D / Total is between 0 and 1
    readout.AmpC = 10;
    readout.AmpD = 10;
    readout.Group = 0; // Group must be valid for the FEN in Ring 0

    // Reset counters before each test
    counters = {};
  }

  void TearDown() override {
    serializers.clear();
  }

  // Helper method to create mock serializers for the instrument
  MockSerializerPtr createMockSerializer() {
    return std::make_shared<MockSerializer>();
  }

  // Helper method to process readouts with optional custom reference times
  // By default adds two readouts (one normal, one late) with reference at 1000s
  // For TOF tests, set addLateReadout=false and provide custom ref times
  void processReadouts(CaenInstrument &Caen, uint32_t refHigh = 1000,
                       esstime::tof_t prevRefHigh = std::nullopt,
                       bool addLateReadout = true) {
    Caen.CaenParser.Result.clear();
    Caen.CaenParser.Result.push_back(readout);

    if (addLateReadout) {
      // Create a second readout with time beyond MaxTOF
      DataParser::CaenReadout lateReadout = readout;
      lateReadout.TimeHigh = 1001;
      lateReadout.TimeLow = 37736786;
      Caen.CaenParser.Result.push_back(lateReadout);
    }

    parser.Packet.Time.setReference(ESSReadout::ESSTime(refHigh, 0));
    if (prevRefHigh.has_value()) {
      parser.Packet.Time.setPrevReference(
          ESSReadout::ESSTime(prevRefHigh.value(), 0));
    }
    Caen.processReadouts();
  }
};

// Test cases below
TEST_F(CaenInstrumentTest, LokiGoodEvent) {
  Settings.CalibFile = LOKI_CALIB;
  CaenInstrument Caen(stats, counters, Settings, parser);

  // Create mock serializer and set it
  auto mockSerializer = createMockSerializer();
  serializers.push_back(mockSerializer);
  Caen.setSerializers(serializers);

  // Good event - addEvent must be called at least once with non-negative ToF
  EXPECT_CALL(*mockSerializer, addEvent(Ge(0), Gt(0))).Times(AtLeast(1));

  processReadouts(Caen);

  // Check results
  EXPECT_EQ(parser.Packet.Time.Counters.TofHigh,
            1); // Expecting 1 event beyond MaxTOF
  EXPECT_EQ(parser.Packet.Time.Counters.TofCount, 1);
}

TEST_F(CaenInstrumentTest, BifrostGoodEvent) {
  Settings.ConfigFile = BIFROST_CONFIG;
  Settings.CalibFile = BIFROST_CALIB;
  Settings.DetectorName = "bifrost";

  // Modify readout values for Bifrost
  readout.AmpA = 30; // For Bifrost, only AmpA and AmpB are used
  readout.AmpB = 30; // Need values that give valid position in [0,1]
  readout.Group = 0; // Must be <= MaxGroup in BifrostGeometry

  CaenInstrument Caen(stats, counters, Settings, parser);
  processReadouts(Caen);

  // Check results
  EXPECT_EQ(parser.Packet.Time.Counters.TofHigh,
            1); // Expecting 1 event beyond MaxTOF
  EXPECT_EQ(parser.Packet.Time.Counters.TofCount, 1);
}

TEST_F(CaenInstrumentTest, BifrostNullCalib) {
  Settings.ConfigFile = BIFROST_CONFIG;
  Settings.CalibFile = "";
  Settings.DetectorName = "bifrost";
  ASSERT_ANY_THROW(CaenInstrument Caen(stats, counters, Settings, parser));
}

TEST_F(CaenInstrumentTest, CspecGoodEvent) {
  Settings.ConfigFile = CSPEC_CONFIG;
  Settings.CalibFile = CSPEC_CALIB;
  Settings.DetectorName = "cspec";
  CaenInstrument Caen(stats, counters, Settings, parser);
  processReadouts(Caen);

  // Check results
  EXPECT_EQ(parser.Packet.Time.Counters.TofHigh,
            1); // Expecting 1 event beyond MaxTOF
  EXPECT_EQ(parser.Packet.Time.Counters.TofCount, 1);
}

TEST_F(CaenInstrumentTest, Tbl3HeGoodEvent) {
  Settings.ConfigFile = TBL3HE_CONFIG;
  Settings.CalibFile = TBL3HE_CALIB;
  Settings.DetectorName = "tbl3he";

  // Modify readout values for Tbl3He
  // Based on Tbl3HeGeometry.cpp, it uses similar validation to Bifrost
  // but with a minimum amplitude check
  readout.FiberId = 20; // For Ring 0

  CaenInstrument Caen(stats, counters, Settings, parser);
  processReadouts(Caen);

  // Check results
  EXPECT_EQ(parser.Packet.Time.Counters.TofHigh,
            1); // Expecting 1 event beyond MaxTOF
  EXPECT_EQ(parser.Packet.Time.Counters.TofCount, 1);
}

// Test that when MaxTOF is 0 in config, default value is used
TEST_F(CaenInstrumentTest, MaxTOFZeroConfig) {
  // Create a temporary config with MaxTofNS = 0
  nlohmann::json testConfig = R"(
  {
    "Detector" : "loki",
    "Resolution" : 512,
    "MaxTOFNS" : 0,
    "GroupsZ" : 4,
    "Banks" : [
       {"Bank" : 0, "ID" : "bank0", "GroupsN" : 56, "YOffset" : 0}
    ],
    
    "Config" : [
    { "Ring" : 0, "Bank" : 0, "FENs" : 16, "FENOffset" :  0}
    ]
  })"_json;

  // Save to a temporary file
  std::string tempConfigFile = "temp_caen_zero_tof.json";
  std::ofstream configFile(tempConfigFile);
  configFile << testConfig.dump(2);
  configFile.close();

  // Use the temporary config
  Settings.ConfigFile = tempConfigFile;
  Settings.CalibFile = LOKI_CALIB;

  // Create the instrument
  CaenInstrument Caen(stats, counters, Settings, parser);

  // Create mock serializer and set it
  auto mockSerializer = createMockSerializer();
  serializers.push_back(mockSerializer);
  Caen.setSerializers(serializers);

  // ToF exceeds limit - addEvent must NOT be called
  EXPECT_CALL(*mockSerializer, addEvent(_, _)).Times(0);

  processReadouts(Caen);

  // Check results with expectation of zero events
  EXPECT_EQ(parser.Packet.Time.Counters.TofHigh, 2);
  EXPECT_EQ(parser.Packet.Time.Counters.TofCount, 0);

  // Clean up
  std::remove(tempConfigFile.c_str());
}

TEST_F(CaenInstrumentTest, PixelError) {
  Settings.CalibFile = LOKI_CALIB;
  CaenInstrument Caen(stats, counters, Settings, parser);

  // Create mock serializer and set it
  auto mockSerializer = createMockSerializer();
  serializers.push_back(mockSerializer);
  Caen.setSerializers(serializers);

  // Create a readout that will cause a pixel error (all amplitudes zero)
  readout.AmpA = 0;
  readout.AmpB = 0;
  readout.AmpC = 0;
  readout.AmpD = 0;

  // Pixel error - addEvent must NOT be called
  EXPECT_CALL(*mockSerializer, addEvent(_, _)).Times(0);

  processReadouts(Caen);

  // Verify that the pixel error was counted through DetectorGeometry
  EXPECT_EQ(Caen.Geom->getBaseCounters().PixelErrors, 1);
  // Verify that Events counter is still 0 (no successful pixel
  // calculation)
  EXPECT_EQ(counters.Events, 0);
}

TEST_F(CaenInstrumentTest, NegativeTOFFallback) {
  Settings.CalibFile = LOKI_CALIB;
  CaenInstrument Caen(stats, counters, Settings, parser);

  auto mockSerializer = createMockSerializer();
  serializers.push_back(mockSerializer);
  Caen.setSerializers(serializers);

  // Readout time is ~1001s, reference=2000s (future), prevReference=1000s
  // TOF vs current pulse is negative, but fallback to prev pulse succeeds
  EXPECT_CALL(*mockSerializer, addEvent(Ge(0), Gt(0))).Times(1);

  processReadouts(Caen, 2000, 1000, false);

  // Verify TOF counters - negative vs current, but fallback succeeded
  EXPECT_EQ(parser.Packet.Time.Counters.TofNegative, 1);
  EXPECT_EQ(parser.Packet.Time.Counters.PrevTofCount, 1);
  EXPECT_EQ(counters.Events, 1);
}

TEST_F(CaenInstrumentTest, NegativePrevTOFError) {
  Settings.CalibFile = LOKI_CALIB;
  CaenInstrument Caen(stats, counters, Settings, parser);

  auto mockSerializer = createMockSerializer();
  serializers.push_back(mockSerializer);
  Caen.setSerializers(serializers);

  // Readout time is ~1001s, reference=2000s, prevReference=1500s
  // TOF is negative vs both pulses - no fallback possible
  EXPECT_CALL(*mockSerializer, addEvent(_, _)).Times(0);

  processReadouts(Caen, 2000, 1500, false);

  // Verify TOF counters - negative vs both pulses
  EXPECT_EQ(parser.Packet.Time.Counters.TofNegative, 1);
  EXPECT_EQ(parser.Packet.Time.Counters.PrevTofNegative, 1);
  EXPECT_EQ(counters.Events, 0);
}

TEST_F(CaenInstrumentTest, HighTOFError) {
  Settings.CalibFile = LOKI_CALIB;
  CaenInstrument Caen(stats, counters, Settings, parser);

  auto mockSerializer = createMockSerializer();
  serializers.push_back(mockSerializer);
  Caen.setSerializers(serializers);

  // Set readout time to 1004s, reference to 1000s
  // TOF = ~4 seconds = 4,000,000,000 ns > INT32_MAX (2,147,483,647)
  readout.TimeHigh = 1004;
  readout.TimeLow = 0;

  // TOF exceeds INT32_MAX - addEvent must NOT be called
  EXPECT_CALL(*mockSerializer, addEvent(_, _)).Times(0);

  processReadouts(Caen, 1000, 0, false);

  // Verify TofHigh counter was incremented
  EXPECT_EQ(parser.Packet.Time.Counters.TofHigh, 1);
  EXPECT_EQ(counters.Events, 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
