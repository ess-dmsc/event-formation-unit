// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <cbm/geometry/Config.h>
#include <common/testutils/TestBase.h>

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

    "Topology" : [
      { "FEN":  0, "Channel": 0, "Type": "TTL", "Source" : "cbm1", "PixelOffset": 0, "PixelRange": 1}
    ]
  }
)"_json;

auto IncorrectFEN = R"(
  {
    "Detector" : "CBM",
    "MonitorRing" : 88,

    "Topology" : [
      { "FEN":  77, "Channel": 0, "Type": "TTL", "Source" : "cbm1", "PixelOffset": 0, "PixelRange": 1}
    ]
  }
)"_json;

auto IncorrectChannel = R"(
  {
    "Detector" : "CBM",
    "MonitorRing" : 88,

    "Topology" : [
      { "FEN":  0, "Channel": 66, "Type": "TTL", "Source" : "cbm1", "PixelOffset": 0, "PixelRange": 1}
    ]
  }
)"_json;

auto DuplicateEntry = R"(
  {
    "Detector" : "CBM",
    "MonitorRing" : 88,

    "Topology" : [
      { "FEN":  10, "Channel": 10, "Type": "TTL", "Source" : "cbm1", "PixelOffset": 0, "PixelRange": 1},
      { "FEN":  10, "Channel": 10, "Type": "TTL", "Source" : "cbm2", "PixelOffset": 0, "PixelRange": 1}
    ]
  }
)"_json;

auto IncorrectType = R"(
  {
    "Detector" : "CBM",
    "MonitorRing" : 11,

    "Topology" : [
      { "FEN":  0, "Channel": 0, "Type": "ESS", "Source" : "cbm1", "PixelOffset": 0, "PixelRange": 1}
    ]
  }
)"_json;

auto ConfigWithTopology = R"(
  {
    "Detector" : "CBM",
    "MonitorRing" : 11,
    "MaxTOFNS" : 1000000000,
    "MaxPulseTimeDiffNS" : 1000000000,

    "Topology" : [
      { "FEN":  0, "Channel": 0, "Type": "TTL", "Source" : "cbm1", "PixelOffset": 0, "PixelRange": 1},
      { "FEN":  0, "Channel": 1, "Type": "TTL", "Source" : "cbm2", "PixelOffset": 0, "PixelRange": 1},
      { "FEN":  1, "Channel": 0, "Type": "IBM", "Source" : "cbm3", "MaxTofBin": 10000, "BinCount": 100},
      { "FEN":  2, "Channel": 0, "Type": "IBM", "Source" : "cbm4", "MaxTofBin": 10000, "BinCount": 100}
    ]
  }
)"_json;
// clang-format on

using namespace cbm;

class CbmConfigTest : public TestBase {
protected:
  Config config{"config.json"}; // dummy filename, not used
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(CbmConfigTest, Constructor) {
  ASSERT_EQ(config.Parms.TypeSubType, ESSReadout::Parser::CBM);
  EXPECT_EQ(config.Parms.MaxTOFNS, 20 * int(1000000000 / 14));
  EXPECT_EQ(config.Parms.MaxPulseTimeDiffNS, 5 * int(1000000000 / 14));
  EXPECT_EQ(config.Parms.MonitorRing, 11);
  EXPECT_TRUE(config.TopologyList.empty());
}

TEST_F(CbmConfigTest, MissingMandatoryField) {
  config.root = MissingDetector;
  ASSERT_ANY_THROW(config.apply());
}

TEST_F(CbmConfigTest, InvalidInstrument) {
  config.root = InvalidDetector;
  ASSERT_ANY_THROW(config.apply());
}

TEST_F(CbmConfigTest, DefaultValues) {
  config.root = DefaultValuesOnly;
  config.apply();
  ASSERT_EQ(config.Parms.TypeSubType, ESSReadout::Parser::CBM);
  EXPECT_EQ(config.Parms.MaxTOFNS, 20 * int(1000000000 / 14));
  EXPECT_EQ(config.Parms.MaxPulseTimeDiffNS, 5 * int(1000000000 / 14));
}

TEST_F(CbmConfigTest, IncorrectFENConfig) {
  try {
    config.root = IncorrectFEN;
    config.apply();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &err) {
    EXPECT_EQ(err.what(), std::string("Entry: 0, Invalid FEN: 77 Max: 11"));
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST_F(CbmConfigTest, IncorrectChannelConfig) {
  try {
    config.root = IncorrectChannel;
    config.apply();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &err) {
    EXPECT_EQ(err.what(), std::string("Entry: 0, Invalid Channel: 66 Max: 11"));
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST_F(CbmConfigTest, DuplicateFENChannelEntry) {
  try {
    config.root = DuplicateEntry;
    config.apply();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &err) {
    EXPECT_EQ(err.what(), std::string("Entry: 1, Duplicate entry for FEN 10 Channel 10"));
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST_F(CbmConfigTest, IncorrectCBMType) {
  try {
    config.root = IncorrectType;
    config.apply();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &err) {
    EXPECT_EQ(err.what(), std::string("Entry: 0, Invalid Type: ESS is not a CBM Type"));
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST_F(CbmConfigTest, FullInstrument) {
  config = Config(CBM_FULL);
  config.loadAndApply();
  ASSERT_EQ(config.Parms.TypeSubType, ESSReadout::Parser::CBM);
  EXPECT_EQ(config.Parms.MaxTOFNS, 1'000'000'000);
  EXPECT_EQ(config.Parms.MaxPulseTimeDiffNS, 1'000'000'000);
  EXPECT_EQ(config.Parms.NumberOfMonitors, 1);
}

TEST_F(CbmConfigTest, TestTopology) {
  config.root = ConfigWithTopology;
  config.apply();
  ASSERT_EQ(config.Parms.TypeSubType, ESSReadout::Parser::CBM);
  EXPECT_EQ(config.Parms.MonitorRing, 11);
  EXPECT_EQ(config.Parms.MaxTOFNS, 1'000'000'000);
  EXPECT_EQ(config.Parms.MaxPulseTimeDiffNS, 1'000'000'000);
  EXPECT_EQ(config.Parms.NumberOfMonitors, 4);

  // Testing topology
  // Test first entry
  EXPECT_EQ(config.TopologyList[0].Type, CbmType::TTL);
  EXPECT_EQ(config.TopologyList[0].Source, "cbm1");
  EXPECT_EQ(config.TopologyList[0].FEN, 0);
  EXPECT_EQ(config.TopologyList[0].Channel, 0);

  // Test second entry
  EXPECT_EQ(config.TopologyList[1].Type, CbmType::TTL);
  EXPECT_EQ(config.TopologyList[1].Source, "cbm2");
  EXPECT_EQ(config.TopologyList[1].FEN, 0);
  EXPECT_EQ(config.TopologyList[1].Channel, 1);

  // Test third entry
  EXPECT_EQ(config.TopologyList[2].Type, CbmType::IBM);
  EXPECT_EQ(config.TopologyList[2].Source, "cbm3");
  EXPECT_EQ(config.TopologyList[2].FEN, 1);
  EXPECT_EQ(config.TopologyList[2].Channel, 0);


  // Test fourth entry
  EXPECT_EQ(config.TopologyList[3].Type, CbmType::IBM);
  EXPECT_EQ(config.TopologyList[3].Source, "cbm4");
  EXPECT_EQ(config.TopologyList[3].FEN, 2);
  EXPECT_EQ(config.TopologyList[3].Channel, 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();
  return RetVal;
}
