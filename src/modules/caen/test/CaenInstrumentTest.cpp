// Copyright (C) 2020-2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <caen/CaenInstrument.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>

using namespace Caen;

// Contrieved but valid configuration file
// The config one panel with 4 x 1 tubes with 7 straws each having a
// resolution of 1 - this gives a total of 28 pixels
std::string LokiConfigFile{"deleteme_loki_instr_config.json"};
std::string LokiConfigStr = R"(
  {
    "Detector": "LoKI",

    "StrawResolution" : 256,

    "PanelConfig" : [
      { "Bank" : 0, "Vertical" :  false,  "TubesZ" : 1, "TubesN" : 1, "StrawOffset" : 0    }
    ]

  }
)";

std::string BifrostConfigFile{"deleteme_bifrost_instr_config.json"};
std::string BifrostConfigStr = R"(
  {
    "Detector": "BIFROST",
    "MaxRing": 4,
    "StrawResolution": 300
  }
)";

/// \brief when used with CalibFile there is a picel count mismatch
std::string Config512File{"deleteme_caen_instr_config_res_512.json"};
std::string Config512Str = R"(
  {
    "Detector": "LoKI",

    "StrawResolution" : 512,

    "PanelConfig" : [
      { "Bank" : 0, "Vertical" :  false,  "TubesZ" : 1, "TubesN" : 1, "StrawOffset" : 0    }
    ]

  }
)";

// Valid calibration for 7 straws
std::string CalibFile{"deleteme_caen_instr_calib.json"};
std::string CalibStr = R"(
  {
    "CaenCalibration" : {
      "ntubes" : 1,
      "nstraws" : 7,
      "resolution" : 256,

      "polynomials" : [
        [0, 0.0, 0.0, 0.0, 0.0],
        [1, 1.0, 0.0, 0.0, 0.0],
        [2, 2.0, 0.0, 0.0, 0.0],
        [3, 0.0, 0.0, 0.0, 0.0],
        [4, 1.0, 0.0, 0.0, 0.0],
        [5, 2.0, 0.0, 0.0, 0.0],
        [6, 2.0, 0.0, 0.0, 0.0]
      ]
    }
  }
)";

class CaenInstrumentTest : public TestBase {
protected:
  struct Counters counters;
  BaseSettings Settings;

  void SetUp() override { Settings.ConfigFile = LokiConfigFile; }
  void TearDown() override {}
};

/** Test cases below */
TEST_F(CaenInstrumentTest, LokiConstructor) {
  Settings.CalibFile = CalibFile;
  CaenInstrument Caen(counters, Settings);
}

TEST_F(CaenInstrumentTest, BifrostConstructor) {
  Settings.ConfigFile = BifrostConfigFile;
  CaenInstrument Caen(counters, Settings);
}

TEST_F(CaenInstrumentTest, PixelMismatch) {
  Settings.ConfigFile = Config512File;
  Settings.CalibFile = CalibFile;
  ASSERT_ANY_THROW(CaenInstrument Caen(counters, Settings));
}

int main(int argc, char **argv) {
  saveBuffer(LokiConfigFile, (void *)LokiConfigStr.c_str(),
             LokiConfigStr.size());
  saveBuffer(BifrostConfigFile, (void *)BifrostConfigStr.c_str(),
             BifrostConfigStr.size());
  saveBuffer(Config512File, (void *)Config512Str.c_str(), Config512Str.size());
  saveBuffer(CalibFile, (void *)CalibStr.c_str(), CalibStr.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(LokiConfigFile);
  deleteFile(BifrostConfigFile);
  deleteFile(Config512File);
  deleteFile(CalibFile);
  return RetVal;
}
