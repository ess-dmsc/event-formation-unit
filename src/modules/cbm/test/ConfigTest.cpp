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
      { "FEN":  0, "Channel": 0, "Type": "TTL", "TypeIndex" : 0}
    ]
  }
)"_json;

auto IncorrectFEN = R"(
  {
    "Detector" : "CBM",
    "MonitorRing" : 88,

    "Topology" : [
      { "FEN":  77, "Channel": 0, "Type": "TTL", "TypeIndex" : 0}
    ]
  }
)"_json;

auto IncorrectChannel = R"(
  {
    "Detector" : "CBM",
    "MonitorRing" : 88,

    "Topology" : [
      { "FEN":  11, "Channel": 66, "Type": "TTL", "TypeIndex" : 0}
    ]
  }
)"_json;

auto DuplicateEntry = R"(
  {
    "Detector" : "CBM",
    "MonitorRing" : 88,

    "Topology" : [
      { "FEN":  10, "Channel": 10, "Type": "TTL", "TypeIndex" : 0},
      { "FEN":  10, "Channel": 10, "Type": "IBM", "TypeIndex" : 1}
    ]
  }
)"_json;

auto IncorrectType = R"(
  {
    "Detector" : "CBM",
    "MonitorRing" : 11,

    "Topology" : [
      { "FEN":  10, "Channel": 10, "Type": "ESS", "TypeIndex" : 0}
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
      { "FEN":  0, "Channel": 0, "Type": "TTL", "TypeIndex" : 0},
      { "FEN":  0, "Channel": 1, "Type": "TTL", "TypeIndex" : 1},
      { "FEN":  1, "Channel": 0, "Type": "IBM", "TypeIndex" : 0},
      { "FEN":  2, "Channel": 0, "Type": "IBM", "TypeIndex" : 1}
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
  EXPECT_EQ(config.MonitorTopology[0][0].isConfigured, false);
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
  EXPECT_EQ(config.MonitorTopology[0][0].Type, CbmType::TTL);
  EXPECT_EQ(config.MonitorTopology[0][0].TypeIndex, 0);
  EXPECT_EQ(config.MonitorTopology[0][1].Type, CbmType::TTL);
  EXPECT_EQ(config.MonitorTopology[0][1].TypeIndex, 1);
  EXPECT_EQ(config.MonitorTopology[1][0].Type, CbmType::IBM);
  EXPECT_EQ(config.MonitorTopology[1][0].TypeIndex, 0);
  EXPECT_EQ(config.MonitorTopology[2][0].Type, CbmType::IBM);
  EXPECT_EQ(config.MonitorTopology[2][0].TypeIndex, 1);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();
  return RetVal;
}
