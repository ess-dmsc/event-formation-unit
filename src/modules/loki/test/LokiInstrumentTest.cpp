// Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <loki/LokiInstrument.h>
#include <test/SaveBuffer.h>
#include <test/TestBase.h>

using namespace Loki;

// Contrieved but valid configuration file
// The config one panel with 4 x 1 tubes with 7 straws each having a
// resolution of 1 - this gives a total of 28 pixels
std::string ConfigFile{"deleteme_loki_instr_config.json"};
std::string ConfigStr = R"(
  {
    "Detector": "LoKI",

    "StrawResolution" : 256,

    "PanelConfig" : [
      { "Bank" : 0, "Vertical" :  false,  "TubesZ" : 1, "TubesN" : 1, "StrawOffset" : 0    }
    ]

  }
)";

// Valid calibration corresponding to TubesZ * TubesN * StrawResolution * 7 = 28
std::string CalibFile{"deleteme_loki_instr_calib.json"};
std::string CalibStr = R"(
  {
    "LokiCalibration":
      {
        "straws" : 7,

        "resolution" : 256,

        "polynomials" :
          [
            {"straw" :    0, "poly" : [0.0, 0.0, 0.0, 0.0]},
            {"straw" :    1, "poly" : [0.0, 0.0, 0.0, 1.0]},
            {"straw" :    2, "poly" : [0.0, 0.0, 0.0, 2.0]},
            {"straw" :    3, "poly" : [0.0, 0.0, 0.0, 0.0]},
            {"straw" :    4, "poly" : [0.0, 0.0, 0.0, 1.0]},
            {"straw" :    5, "poly" : [0.0, 0.0, 0.0, 2.0]},
            {"straw" :    6, "poly" : [0.0, 0.0, 0.0, 2.0]}
          ]
      }
  }
)";


class LokiInstrumentTest : public TestBase {
protected:
  struct Counters counters;
  LokiSettings ModuleSettings;

  void SetUp() override {
    ModuleSettings.ConfigFile = ConfigFile;
  }
  void TearDown() override {}
};

/** Test cases below */
TEST_F(LokiInstrumentTest, Constructor) {
  ModuleSettings.CalibFile = CalibFile;
  LokiInstrument Loki(counters, ModuleSettings);
  ASSERT_EQ(Loki.LokiConfiguration.getMaxPixel(), 7 * 256);
}

int main(int argc, char **argv) {
  saveBuffer(ConfigFile, (void *)ConfigStr.c_str(), ConfigStr.size());
  saveBuffer(CalibFile, (void *)CalibStr.c_str(), CalibStr.size());
  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();
  return RetVal;
}
