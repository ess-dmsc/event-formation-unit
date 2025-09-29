// Copyright (C) 2020 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/Statistics.h>
#include <common/readout/ess/Parser.h>
#include <readout/DataParser.h>
#include <caen/CaenInstrument.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>

using namespace Caen;

class CaenInstrumentTest : public TestBase {
protected:
  struct CaenCounters counters;
  Statistics stats;
  ESSReadout::Parser parser{stats};
  BaseSettings Settings;
  DataParser::CaenReadout readout;

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

  void TearDown() override {}

  // Helper method to process readouts without assertions
  void processReadouts(CaenInstrument &Caen) {
    // Clear any existing results and add our test readout
    Caen.CaenParser.Result.clear();

    // Add our first test readout
    Caen.CaenParser.Result.push_back(readout);

    // Create a second readout with time beyond MaxTOF
    DataParser::CaenReadout lateReadout = readout;
    // This time high and time low are set to simulate a late readout
    // that exceeds the MaxTOF threshold, ~1.4 seconds later then reference time
    lateReadout.TimeHigh = 1001;
    lateReadout.TimeLow = 37736786;

    // Add the late readout
    Caen.CaenParser.Result.push_back(lateReadout);

    // Set reference time
    parser.Packet.Time.setReference(ESSReadout::ESSTime(1000, 0));

    // Process readouts
    Caen.processReadouts();
  }
};

// Test cases below
TEST_F(CaenInstrumentTest, LokiInstrumentReadoutTest) {
  Settings.CalibFile = LOKI_CALIB;
  CaenInstrument Caen(stats, counters, Settings, parser);

  processReadouts(Caen);

  // Check results
  EXPECT_EQ(parser.Packet.Time.Counters.TofHigh,
            1); // Expecting 1 event beyond MaxTOF
  EXPECT_EQ(parser.Packet.Time.Counters.TofCount, 1);
}

TEST_F(CaenInstrumentTest, BifrostInstrumentReadoutTest) {
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

TEST_F(CaenInstrumentTest, BifrostInstrumentNullCalibTest) {
  Settings.ConfigFile = BIFROST_CONFIG;
  Settings.CalibFile = "";
  Settings.DetectorName = "bifrost";
  ASSERT_ANY_THROW(CaenInstrument Caen(stats, counters, Settings, parser));
}

TEST_F(CaenInstrumentTest, CspecInstrumentReadoutTest) {
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

TEST_F(CaenInstrumentTest, Tbl3HeInstrumentReadoutTest) {
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
TEST_F(CaenInstrumentTest, CheckMaxTOFZeroApplied) {
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

  processReadouts(Caen);

  // Check results with expectation of zero events
  EXPECT_EQ(parser.Packet.Time.Counters.TofHigh, 2);
  EXPECT_EQ(parser.Packet.Time.Counters.TofCount, 0);

  // Clean up
  std::remove(tempConfigFile.c_str());
}

TEST_F(CaenInstrumentTest, CountPixelErrors) {
  Settings.CalibFile = LOKI_CALIB;
  CaenInstrument Caen(stats, counters, Settings, parser);

  // Create a readout that will cause a pixel error (all amplitudes zero)
  readout.AmpA = 0;
  readout.AmpB = 0;
  readout.AmpC = 0;
  readout.AmpD = 0;

  processReadouts(Caen);

  // Verify that the pixel error was counted through DetectorGeometry
  EXPECT_EQ(Caen.Geom->getBaseCounters().PixelErrors, 1);
  // Verify that Events counter is still 0 (no successful pixel calculation)
  EXPECT_EQ(counters.Events, 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
