// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <cbm/geometry/Config.h>

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
    "Detector" : "CBM"
  }
)"_json;

auto RingAndFEN = R"(
  {
    "Detector" : "CBM",
    "MonitorRing" : 88,
    "MonitorFEN" : 77
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
  ASSERT_EQ(config.Parms.MaxTOFNS, 20 * int(1000000000 / 14));
  ASSERT_EQ(config.Parms.MaxPulseTimeDiffNS, 5 * int(1000000000 / 14));
  ASSERT_EQ(config.Parms.MonitorRing, 11);
  ASSERT_EQ(config.Parms.MonitorFEN, 0);
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
  ASSERT_EQ(config.Parms.MaxTOFNS, 20 * int(1000000000 / 14));
  ASSERT_EQ(config.Parms.MaxPulseTimeDiffNS, 5 * int(1000000000 / 14));
}

TEST_F(CbmConfigTest, RingAndFENConfig) {
  config.root = RingAndFEN;
  config.apply();
  ASSERT_EQ(config.Parms.MonitorRing, 88);
  ASSERT_EQ(config.Parms.MonitorFEN, 77);
}

TEST_F(CbmConfigTest, FullInstrument) {
  config = Config(CBM_FULL);
  config.loadAndApply();
  ASSERT_EQ(config.Parms.TypeSubType, ESSReadout::Parser::CBM);
  ASSERT_EQ(config.Parms.MaxTOFNS, 1'000'000'000);
  ASSERT_EQ(config.Parms.MaxPulseTimeDiffNS, 1'000'000'000);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();
  return RetVal;
}
