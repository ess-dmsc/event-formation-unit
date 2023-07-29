// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <loki/geometry/LokiConfig.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>


// Good configuration file
auto ValidConfig = R"(
  {
    "Detector": "loki",

    "Resolution" : 512,
    "GroupsZ" : 4,

    "ReadoutConstDelayNS" : 0,
    "MaxPulseTimeNS" : 357142855,
    "MaxTOFNS" : 1000000000,

    "Banks" : [
       {"Bank" : 0, "ID" : "bank0", "GroupsN" : 56, "YOffset" : 0},
       {"Bank" : 1, "ID" : "bank1", "GroupsN" : 16, "YOffset" : 1568},
       {"Bank" : 2, "ID" : "bank2", "GroupsN" : 12, "YOffset" : 2016},
       {"Bank" : 3, "ID" : "bank3", "GroupsN" : 16, "YOffset" : 2352},
       {"Bank" : 4, "ID" : "bank4", "GroupsN" : 12, "YOffset" : 2800},
       {"Bank" : 5, "ID" : "bank5", "GroupsN" : 28, "YOffset" : 3136},
       {"Bank" : 6, "ID" : "bank6", "GroupsN" : 32, "YOffset" : 3920},
       {"Bank" : 7, "ID" : "bank7", "GroupsN" : 20, "YOffset" : 4816},
       {"Bank" : 8, "ID" : "bank8", "GroupsN" : 32, "YOffset" : 5376}
    ],

    "Config" : [
      { "Ring" : 0, "Bank" : 0, "FENs" : 16, "FENOffset" :  0},
      { "Ring" : 1, "Bank" : 0, "FENs" : 12, "FENOffset" : 16},
      { "Ring" : 2, "Bank" : 1, "FENs" :  8, "FENOffset" :  0},
      { "Ring" : 3, "Bank" : 2, "FENs" :  6, "FENOffset" :  0},
      { "Ring" : 4, "Bank" : 3, "FENs" :  8, "FENOffset" :  0},
      { "Ring" : 5, "Bank" : 4, "FENs" :  6, "FENOffset" :  0},
      { "Ring" : 6, "Bank" : 5, "FENs" : 14, "FENOffset" :  0},
      { "Ring" : 7, "Bank" : 6, "FENs" : 16, "FENOffset" :  0},
      { "Ring" : 8, "Bank" : 7, "FENs" : 10, "FENOffset" :  0},
      { "Ring" : 9, "Bank" : 8, "FENs" : 16, "FENOffset" :  0}
    ]
  }
)"_json;

using namespace Caen;

class LokiConfigTest : public TestBase {
protected:
  LokiConfig config;
  void SetUp() override {}
  void TearDown() override {}
};


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

TEST_F(LokiConfigTest, ValidConfig) {
  config.root = ValidConfig;
  config.parseConfig();
  ASSERT_EQ(config.Parms.Resolution, 512);
}



TEST_F(LokiConfigTest, GetGlobalGroup) {
  config.root = ValidConfig;
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
