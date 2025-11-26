// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <cbm/geometry/Config.h>
#include <common/testutils/TestBase.h>

#include <filesystem>

using std::filesystem::path;

// clang-format off
auto MissingDetector = R"(
  {
    "Instrument" : "Freia"
  }
)"_json;

auto InvalidDetector = R"(
  {
    "Detector" : "Freia"
  }
)"_json;

auto DefaultValuesOnly = R"(
  {
    "Detector" : "CBM",
    "MaxFENId" : 1,

    "Topology" : [
      { "FEN":  0, "Channel": 0, "Type": "EVENT_0D", "Source" : "cbm1", "PixelOffset": 0}
    ]
  }
)"_json;

auto IncorrectFEN = R"(
  {
    "Detector" : "CBM",
    "MonitorRing" : 88,
    "MaxFENId" : 11,

    "Topology" : [
      { "FEN":  12, "Channel": 0, "Type": "EVENT_0D", "Source" : "cbm1", "PixelOffset": 0}
    ]
  }
)"_json;

auto FENIdEdgeCase = R"(
  {
    "Detector" : "CBM",
    "MonitorRing" : 88,
    "MaxFENId" : 11,

    "Topology" : [
      { "FEN":  11, "Channel": 0, "Type": "EVENT_0D", "Source" : "cbm1", "PixelOffset": 0}
    ]
  }
)"_json;

auto NoMaxFENId = R"(
  {
    "Detector" : "CBM",
    "MonitorRing" : 88,

    "Topology" : [
      { "FEN":  11, "Channel": 0, "Type": "EVENT_0D", "Source" : "cbm1", "PixelOffset": 0}
    ]
  }
)"_json;

auto DuplicateEntry = R"(
  {
    "Detector" : "CBM",
    "MonitorRing" : 88,
    "MaxFENId" : 11,

    "Topology" : [
      { "FEN":  10, "Channel": 10, "Type": "EVENT_0D", "Source" : "cbm1", "PixelOffset": 0},
      { "FEN":  10, "Channel": 10, "Type": "EVENT_0D", "Source" : "cbm2", "PixelOffset": 0}
    ]
  }
)"_json;

auto IncorrectType = R"(
  {
    "Detector" : "CBM",
    "MonitorRing" : 11,
    "MaxFENId" : 1,

    "Topology" : [
      { "FEN":  0, "Channel": 0, "Type": "ESS", "Source" : "cbm1", "PixelOffset": 0}
    ]
  }
)"_json;

auto NoTopology = R"(
  {
    "Detector" : "CBM",
    "MonitorRing" : 11,
    "MaxFENId" : 1
  }
)"_json;

auto ConfigWithTopology = R"(
  {
    "Detector" : "CBM",
    "MonitorRing" : 11,
    "MaxTOFNS" : 1000000000,
    "MaxPulseTimeDiffNS" : 1000000000,
    "MaxFENId" : 2,

    "Topology" : [
      { "FEN":  0, "Channel": 0, "Type": "EVENT_0D", "Source" : "cbm1", "PixelOffset": 0},
      { "FEN":  0, "Channel": 1, "Type": "EVENT_0D", "Source" : "cbm2", "PixelOffset": 0},
      { "FEN":  1, "Channel": 0, "Type": "EVENT_0D", "Source" : "cbm3", "PixelOffset": 0},
      { "FEN":  2, "Channel": 0, "Type": "IBM", "Source" : "cbm4", "MaxTofBin": 10000, "BinCount": 100},
      { "FEN":  0, "Channel": 2, "Type": "IBM", "Source" : "cbm5", "MaxTofBin": 10000, "BinCount": 100},
      { "FEN":  2, "Channel": 1, "Type": "IBM", "Source" : "cbm6", "MaxTofBin": 10000, "BinCount": 100},
      { "FEN":  2, "Channel": 2, "Type": "EVENT_2D", "Source" : "cbm7", "Width": 512, "Height": 512},
      { "FEN":  1, "Channel": 2, "Type": "EVENT_2D", "Source" : "cbm8", "Width": 512, "Height": 512},
      { "FEN":  1, "Channel": 1, "Type": "EVENT_2D", "Source" : "cbm9", "Width": 512, "Height": 512}
    ]
  }
)"_json;

// clang-format on

using namespace cbm;

class CbmConfigTest : public TestBase {
protected:
  Config config{"config.json"}; // dummy filename, not used

  inline static path FullConfigFile{""};

  void SetUp() override {

    // Get base test dir
    path TestDir = path(__FILE__).parent_path();
    // Define test files
    FullConfigFile = TestDir / path("cbm_config_test.json");
  }

  void TearDown() override {}
};

TEST_F(CbmConfigTest, Constructor) {
  ASSERT_EQ(config.CbmParms.TypeSubType, DetectorType::CBM);
  EXPECT_EQ(config.CbmParms.MaxTOFNS, 20 * int(1000000000 / 14));
  EXPECT_EQ(config.CbmParms.MaxPulseTimeDiffNS, 5 * int(1000000000 / 14));
  EXPECT_EQ(config.CbmParms.MonitorRing, 11);
  EXPECT_EQ(config.TopologyMapPtr, nullptr);
}

TEST_F(CbmConfigTest, MissingMandatoryField) {
  config.setRoot(MissingDetector);
  ASSERT_ANY_THROW(config.apply());
}

TEST_F(CbmConfigTest, InvalidInstrument) {
  config.setRoot(InvalidDetector);
  ASSERT_ANY_THROW(config.apply());
}

TEST_F(CbmConfigTest, DefaultValues) {
  config.setRoot(DefaultValuesOnly);
  config.apply();
  ASSERT_EQ(config.CbmParms.TypeSubType, DetectorType::CBM);
  EXPECT_EQ(config.CbmParms.MaxTOFNS, 20 * int(1000000000 / 14));
  EXPECT_EQ(config.CbmParms.MaxPulseTimeDiffNS, 5 * int(1000000000 / 14));
}

TEST_F(CbmConfigTest, IncorrectFENConfig) {
  try {
    config.setRoot(IncorrectFEN);
    config.apply();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &err) {
    EXPECT_EQ(err.what(), std::string("Entry: 0, Invalid FEN: 12 Max: 11"));
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST_F(CbmConfigTest, FenIdEdgeTestCase) {
  config.setRoot(FENIdEdgeCase);
  ASSERT_NO_THROW(config.apply());
}

TEST_F(CbmConfigTest, NoMaxFENIdSpecified) {
  try {
    config.setRoot(NoMaxFENId);
    config.apply();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &err) {
    EXPECT_EQ(err.what(), std::string("JSON config - error: The requested key 'MaxFENId' does not exist"));
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST_F(CbmConfigTest, DuplicateFENChannelEntry) {
  try {
    config.setRoot(DuplicateEntry);
    config.apply();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &err) {
    EXPECT_EQ(err.what(),
              std::string("Entry: 1, Duplicate entry for FEN 10 Channel 10"));
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST_F(CbmConfigTest, IncorrectCBMType) {
  try {
    config.setRoot(IncorrectType);
    config.apply();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &err) {
    EXPECT_EQ(err.what(),
              std::string("Entry: 0, Invalid Type: ESS is not a CBM Type"));
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST_F(CbmConfigTest, NoTopology) {
  try {
    config.setRoot(NoTopology);
    config.apply();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &err) {
    EXPECT_EQ(err.what(),
              std::string("No 'Topology' section found in the "
                          "configuration. Cannot setup Beam Monitors"));
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST_F(CbmConfigTest, LoadFileFullInstrument) {
  config = Config(FullConfigFile);
  config.loadAndApply();
  ASSERT_EQ(config.CbmParms.TypeSubType, DetectorType::CBM);
  EXPECT_EQ(config.CbmParms.MonitorRing, 11);
  EXPECT_EQ(config.CbmParms.MaxTOFNS, 1'000'000'000);
  EXPECT_EQ(config.CbmParms.MaxPulseTimeDiffNS, 1'000'000'000);
  EXPECT_EQ(config.CbmParms.MaxFENId, 2);
  EXPECT_EQ(config.CbmParms.NumberOfMonitors, 3);
}

//Test that CBM IBM get default values for aggregated frames
//and for aggregation mode
TEST_F(CbmConfigTest, TestDefaultAggregateFramesConfig) {
    auto ConfigJson = R"(
    {
      "Detector"           : "CBM",
      "MonitorRing"        : 11,
      "MaxTOFNS"           : 1000000000,
      "MaxPulseTimeDiffNS" : 1000000000,
      "MaxFENId"           : 2,

      "Topology" : [
        { 
          "FEN":  1, 
          "Channel": 1, 
          "Type": "IBM", 
          "Source" : "cbm1", 
          "MaxTofBin": 10000, 
          "BinCount": 100 
        }
      ]
    }
  )"_json;
  
  config.setRoot(ConfigJson);
  config.apply();
  auto *TopologyEntry = config.TopologyMapPtr->get(1, 1);
  EXPECT_EQ(TopologyEntry->Type, CbmType::IBM);
  EXPECT_EQ(TopologyEntry->Source, "cbm1");
  EXPECT_EQ(TopologyEntry->FEN, 1);
  EXPECT_EQ(TopologyEntry->Channel, 1);
  EXPECT_EQ(TopologyEntry->maxTofBin, 10000);
  EXPECT_EQ(TopologyEntry->BinCount, 100);
  EXPECT_EQ(TopologyEntry->AggregatedFrames, 1);
  EXPECT_EQ(TopologyEntry->AggregationMode, (int)AggregationType::SUM);
}

//Test that CBM IBM get aggregated frames and mode from json
TEST_F(CbmConfigTest, TestOverrideAggregateFramesConfig) {
    auto ConfigJson = R"(
    {
      "Detector" : "CBM",
      "MonitorRing" : 11,
      "MaxTOFNS" : 1000000000,
      "MaxPulseTimeDiffNS" : 1000000000,
      "MaxFENId" : 2,

      "Topology" : [
        { 
          "FEN":  1, 
          "Channel": 1, 
          "Type": "IBM", 
          "Source" : "cbm1", 
          "MaxTofBin": 10000, 
          "BinCount": 100,
          "AggregatedFrames": 20, 
          "AggregationMode": 1 
        }
      ]
    }
  )"_json;
  
  config.setRoot(ConfigJson);
  config.apply();
  auto *TopologyEntry = config.TopologyMapPtr->get(1, 1);
  EXPECT_EQ(TopologyEntry->Type, CbmType::IBM);
  EXPECT_EQ(TopologyEntry->Source, "cbm1");
  EXPECT_EQ(TopologyEntry->FEN, 1);
  EXPECT_EQ(TopologyEntry->Channel, 1);
  EXPECT_EQ(TopologyEntry->maxTofBin, 10000);
  EXPECT_EQ(TopologyEntry->BinCount, 100);
  EXPECT_EQ(TopologyEntry->AggregatedFrames, 20);
  EXPECT_EQ(TopologyEntry->AggregationMode, (int)AggregationType::AVG);
}

//Test that CBM IBM get aggregated frames and mode from json
TEST_F(CbmConfigTest, TestMalformedAggregateFramesConfig) {
    auto ConfigJson = R"(
    {
      "Detector" : "CBM",
      "MonitorRing" : 11,
      "MaxTOFNS" : 1000000000,
      "MaxPulseTimeDiffNS" : 1000000000,
      "MaxFENId" : 2,

      "Topology" : [
        {
          "FEN":  1, 
          "Channel": 1, 
          "Type": "IBM", 
          "Source" : "cbm1", 
          "MaxTofBin": 10000, 
          "BinCount": 100,
          "AggregatedFrames": 20, 
          "AggregationMode": 2 
        }
      ]
    }
  )"_json;
  
  config.setRoot(ConfigJson);
  EXPECT_THROW(config.apply(), std::runtime_error);
}


TEST_F(CbmConfigTest, TestCBM2DErrorConfig) {
    auto ConfigJson = R"(
    {
      "Detector" : "CBM",
      "MonitorRing" : 11,
      "MaxTOFNS" : 1000000000,
      "MaxPulseTimeDiffNS" : 1000000000,
      "MaxFENId" : 2,

      "Topology" : [
        { "FEN":  1, "Channel": 1, "Type": "EVENT_2D", "Source" : "cbm9", "Width": 0, "Height": 0}
      ]
    }
  )"_json;
  
  config.setRoot(ConfigJson);

  //Widths are outside valid range
  config["Topology"][0]["Width"] = -1;
  config["Topology"][0]["Height"] = 512;
  EXPECT_THROW(config.apply(), std::runtime_error);
  config["Topology"][0]["Width"] = 65636;
  config["Topology"][0]["Height"] = 321;
  EXPECT_THROW(config.apply(), std::runtime_error);

  //Heights are outside valid range
  config["Topology"][0]["Width"] = 312;
  config["Topology"][0]["Height"] = -1;
  EXPECT_THROW(config.apply(), std::runtime_error);
  config["Topology"][0]["Width"] = 123;
  config["Topology"][0]["Height"] = 65636;
  EXPECT_THROW(config.apply(), std::runtime_error);

  // Valid values.
  config["Topology"][0]["Width"] = 256;
  config["Topology"][0]["Height"] = 356;
  EXPECT_NO_THROW(config.apply());
}

TEST_F(CbmConfigTest, TestTopology) {
  config.setRoot(ConfigWithTopology);
  config.apply();
  ASSERT_EQ(config.CbmParms.TypeSubType, DetectorType::CBM);
  EXPECT_EQ(config.CbmParms.MonitorRing, 11);
  EXPECT_EQ(config.CbmParms.MaxTOFNS, 1'000'000'000);
  EXPECT_EQ(config.CbmParms.MaxPulseTimeDiffNS, 1'000'000'000);
  EXPECT_EQ(config.CbmParms.NumberOfMonitors, 9);

  // Testing topology
  // Test first entry
  auto *TopologyEntry = config.TopologyMapPtr->get(0, 0);
  EXPECT_EQ(TopologyEntry->Type, CbmType::EVENT_0D);
  EXPECT_EQ(TopologyEntry->Source, "cbm1");
  EXPECT_EQ(TopologyEntry->FEN, 0);
  EXPECT_EQ(TopologyEntry->Channel, 0);

  // Test second entry
  TopologyEntry = config.TopologyMapPtr->get(0, 1);
  EXPECT_EQ(TopologyEntry->Type, CbmType::EVENT_0D);
  EXPECT_EQ(TopologyEntry->Source, "cbm2");
  EXPECT_EQ(TopologyEntry->FEN, 0);
  EXPECT_EQ(TopologyEntry->Channel, 1);

  // Test third entry
  TopologyEntry = config.TopologyMapPtr->get(1, 0);
  EXPECT_EQ(TopologyEntry->Type, CbmType::EVENT_0D);
  EXPECT_EQ(TopologyEntry->Source, "cbm3");
  EXPECT_EQ(TopologyEntry->FEN, 1);
  EXPECT_EQ(TopologyEntry->Channel, 0);

  // Test fourth entry
  TopologyEntry = config.TopologyMapPtr->get(2, 0);
  EXPECT_EQ(TopologyEntry->Type, CbmType::IBM);
  EXPECT_EQ(TopologyEntry->Source, "cbm4");
  EXPECT_EQ(TopologyEntry->FEN, 2);
  EXPECT_EQ(TopologyEntry->Channel, 0);

  // Test fifth entry
  TopologyEntry = config.TopologyMapPtr->get(0, 2);
  EXPECT_EQ(TopologyEntry->Type, CbmType::IBM);
  EXPECT_EQ(TopologyEntry->Source, "cbm5");
  EXPECT_EQ(TopologyEntry->FEN, 0);
  EXPECT_EQ(TopologyEntry->Channel, 2);

  // Test sixth entry
  TopologyEntry = config.TopologyMapPtr->get(2, 1);
  EXPECT_EQ(TopologyEntry->Type, CbmType::IBM);
  EXPECT_EQ(TopologyEntry->Source, "cbm6");
  EXPECT_EQ(TopologyEntry->FEN, 2);
  EXPECT_EQ(TopologyEntry->Channel, 1);

  // Test seventh entry
  TopologyEntry = config.TopologyMapPtr->get(2, 2);
  EXPECT_EQ(TopologyEntry->Type, CbmType::EVENT_2D);
  EXPECT_EQ(TopologyEntry->Source, "cbm7");
  EXPECT_EQ(TopologyEntry->FEN, 2);
  EXPECT_EQ(TopologyEntry->Channel, 2);

  // Test eight entry
  TopologyEntry = config.TopologyMapPtr->get(1, 2);
  EXPECT_EQ(TopologyEntry->Type, CbmType::EVENT_2D);
  EXPECT_EQ(TopologyEntry->Source, "cbm8");
  EXPECT_EQ(TopologyEntry->FEN, 1);
  EXPECT_EQ(TopologyEntry->Channel, 2);

  // Test ninth entry
  TopologyEntry = config.TopologyMapPtr->get(1, 1);
  EXPECT_EQ(TopologyEntry->Type, CbmType::EVENT_2D);
  EXPECT_EQ(TopologyEntry->Source, "cbm9");
  EXPECT_EQ(TopologyEntry->FEN, 1);
  EXPECT_EQ(TopologyEntry->Channel, 1);

}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();
  return RetVal;
}
