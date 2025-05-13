// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <loki/geometry/LokiConfig.h>
#include <common/testutils/TestBase.h>

using namespace Caen;


class LokiConfigTest : public TestBase {
protected:
  LokiConfig config{LOKI_CONFIG};
  void SetUp() override {
    ASSERT_EQ(config.Parms.Resolution, 0); // check one var is uninitialised
  }
  void TearDown() override {}
};


TEST_F(LokiConfigTest, ParseConfig) {
  ASSERT_NO_THROW(config.parseConfig());
  ASSERT_EQ(config.Parms.Resolution, 512);
  ASSERT_EQ(config.Parms.ConfiguredBanks, 9);
  ASSERT_EQ(config.Parms.ConfiguredRings, 10);
}

TEST_F(LokiConfigTest, NoDetectorKey) {
  json_change_key(config.root(), "Detector", "InvalidDetector");
  ASSERT_ANY_THROW(config.parseConfig());
}

TEST_F(LokiConfigTest, BadDetectorName) {
  config["Detector"] = "nosuchdetector";
  ASSERT_ANY_THROW(config.parseConfig());
}

TEST_F(LokiConfigTest, BadBank) {
  config["Banks"][0]["Bank"] = 200;
  ASSERT_NO_THROW(config.parseConfig());
  ASSERT_EQ(config.Parms.ConfiguredBanks, 8);
}

TEST_F(LokiConfigTest, BadRing) {
  config["Config"][0]["Ring"] = 200;
  ASSERT_NO_THROW(config.parseConfig());
  ASSERT_EQ(config.Parms.ConfiguredRings, 9);
}

TEST_F(LokiConfigTest, Constructor) {
  for (int i = 0; i < config.Parms.NumRings; i++) {
    ASSERT_EQ(config.Parms.Rings[i].Bank, -1);
    ASSERT_EQ(config.Parms.Rings[i].FENs, 0);
    ASSERT_EQ(config.Parms.Rings[i].FENOffset, 0);
  }
  for (int i = 0; i < config.Parms.NumBanks; i++) {
    ASSERT_EQ(config.Parms.Banks[i].GroupsN, 0);
    ASSERT_EQ(config.Parms.Banks[i].YOffset, 0);
  }
  ASSERT_EQ(config.Parms.Resolution, 0);
}

TEST_F(LokiConfigTest, GetGlobalGroup) {
  config.parseConfig();
  //  Ring, FEN, (Local)Group     R   F  LG
  // Validating partitioned bank0
  ASSERT_EQ(config.getGlobalGroup(0,  0, 0), 0);
  ASSERT_EQ(config.getGlobalGroup(0,  0, 4), 1);
  ASSERT_EQ(config.getGlobalGroup(0, 15, 4), 31);
  ASSERT_EQ(config.getGlobalGroup(1,  0, 0), 32);
  ASSERT_EQ(config.getGlobalGroup(1,  0, 4), 33);
  ASSERT_EQ(config.getGlobalGroup(1, 11, 4), 55);
  ASSERT_EQ(config.getGlobalGroup(1, 11, 7), 223);

  ASSERT_EQ(config.getGlobalGroup(0,  0, 1),   0 + 56);
  ASSERT_EQ(config.getGlobalGroup(0, 15, 5),  31 + 56);
  ASSERT_EQ(config.getGlobalGroup(1,  0, 1),  32 + 56);
  ASSERT_EQ(config.getGlobalGroup(1, 11, 5),  55 + 56);

  // Ad hoc checking
  ASSERT_EQ(config.getGlobalGroup(2,  0, 0), 224);
  ASSERT_EQ(config.getGlobalGroup(2,  0, 1), 224 + 16);

  // Last Group
  ASSERT_EQ(config.getGlobalGroup(9, 15, 7), 895);

}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
