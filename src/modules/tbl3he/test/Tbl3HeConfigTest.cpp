// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <tbl3he/geometry/Tbl3HeConfig.h>
#include <common/testutils/TestBase.h>

using namespace Caen;

auto ValidConfig = R"(
  {
    "Detector": "tbl3he",
    "MaxRing": 11,
    "Resolution": 100,
    "MaxGroup": 7,
    "MaxPulseTimeNS" : 71428600,
    "MaxTOFNS" : 500000000,

    "NumOfFENs" : 2,

    "MinValidAmplitude" : 2,

    "Topology" : [
       {"Ring" : 10, "FEN" : 0, "Bank" : 0},
       {"Ring" :  9, "FEN" : 0, "Bank" : 1}
    ]
  }
)"_json;


class Tbl3HeConfigTest : public TestBase {
protected:
  Tbl3HeConfig config;
  void SetUp() override {}
  void TearDown() override {}
};


TEST_F(Tbl3HeConfigTest, Constructor) {
  ASSERT_EQ(config.Parms.Resolution, 0);
  ASSERT_EQ(config.Parms.MaxPulseTimeNS, 0);
  ASSERT_EQ(config.Parms.MaxTOFNS, 0);
  ASSERT_EQ(config.Parms.MinValidAmplitude, 0);
  ASSERT_EQ(config.Parms.NumOfFENs, 0);
  ASSERT_EQ(config.Parms.MaxGroup, 0);
}


TEST_F(Tbl3HeConfigTest, BadJsonFile) {
  ASSERT_THROW(Tbl3HeConfig MyConfig("nofile.json"), std::runtime_error);
}


TEST_F(Tbl3HeConfigTest, BadDetectorName) {
  config.setRoot(ValidConfig);
  config["Detector"] = "mybad";
  ASSERT_THROW(config.parseConfig(), std::runtime_error);
}


TEST_F(Tbl3HeConfigTest, ValidConfig) {
  config.setRoot(ValidConfig);
  ASSERT_NO_THROW(config.parseConfig());
  ASSERT_EQ(config.Parms.Resolution, 100);
  ASSERT_EQ(config.Parms.MaxPulseTimeNS, 71428600);
  ASSERT_EQ(config.Parms.MaxTOFNS, 500000000);
  ASSERT_EQ(config.Parms.MinValidAmplitude, 2);
  ASSERT_EQ(config.Parms.NumOfFENs, 2);
  ASSERT_EQ(config.Parms.MaxGroup, 7);
}


TEST_F(Tbl3HeConfigTest, InvalidConfigMinRing) {
  config.setRoot(ValidConfig);
  config["Topology"][0]["Ring"] = -1; // outside interval 0 - 11
  ASSERT_THROW(config.parseConfig(), std::runtime_error);
}

TEST_F(Tbl3HeConfigTest, InvalidConfigMaxRing) {
  config.setRoot(ValidConfig);
  config["Topology"][0]["Ring"] = 12; // outside interval 0 - 11
  ASSERT_THROW(config.parseConfig(), std::runtime_error);
}

TEST_F(Tbl3HeConfigTest, InvalidConfigNumOfFENs) {
  config.setRoot(ValidConfig);
  config["NumOfFENs"] = 3;
  ASSERT_THROW(config.parseConfig(), std::runtime_error);
}

TEST_F(Tbl3HeConfigTest, InvalidConfigDuplicateRingFEN) {
  config.setRoot(ValidConfig);
  config["Topology"][0]["Ring"] = 9;
  ASSERT_THROW(config.parseConfig(), std::runtime_error);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
