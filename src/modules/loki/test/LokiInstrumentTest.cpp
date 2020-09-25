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

    "StrawResolution" : 1,

    "PanelConfig" : [
      { "Bank" : 0, "Vertical" :  false,  "TubesZ" : 4, "TubesN" : 1, "StrawOffset" : 0    }
    ]

  }
)";

// Valid calibration corresponding to TubesZ * TubesN * StrawResolution * 7 = 28
std::string CalibFile{"deleteme_loki_instr_calib.json"};
std::string CalibStr = R"(
  {
    "LokiCalibration":
      {
        "Mapping":[ 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
                   10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                   20, 21, 22, 23, 24, 25, 26, 27, 28
                  ]
      }
  }
)";

// Illegal mapping of 0 to nonzero pixel value
std::string CalibInvalidMappingFile{"deleteme_loki_instr_calib_bad_map.json"};
std::string CalibInvalidMappingStr = R"(
  {
    "LokiCalibration":
      {
        "Mapping":[ 99,  1,  2,  3,  4,  5,  6,  7,  8,  9,
                    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                    20, 21, 22, 23, 24, 25, 26, 27, 28
                  ]
      }
  }
)";


// Too few pixels to match Config
std::string CalibInvalidPixelFile{"deleteme_loki_instr_calib_bad_pixel.json"};
std::string CalibInvalidPixelStr = R"(
  {
    "LokiCalibration":
      {
        "Mapping":[ 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
                   10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                   20, 21, 22, 23, 24, 25, 26, 27
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
  ASSERT_EQ(Loki.LokiConfiguration.getMaxPixel(), 28);
}

TEST_F(LokiInstrumentTest, InvalidMapping) {
  ModuleSettings.CalibFile = CalibInvalidMappingFile;
  ASSERT_ANY_THROW(LokiInstrument Loki(counters, ModuleSettings));
  deleteFile(CalibInvalidMappingFile);
}

TEST_F(LokiInstrumentTest, InvalidNumberOfPixels) {
  ModuleSettings.CalibFile = CalibInvalidPixelFile;
  ASSERT_ANY_THROW(LokiInstrument Loki(counters, ModuleSettings));
  deleteFile(CalibInvalidPixelFile);
}

int main(int argc, char **argv) {
  saveBuffer(ConfigFile, (void *)ConfigStr.c_str(), ConfigStr.size());
  saveBuffer(CalibFile, (void *)CalibStr.c_str(), CalibStr.size());
  saveBuffer(CalibInvalidMappingFile, (void *)CalibInvalidMappingStr.c_str(), CalibInvalidMappingStr.size());
  saveBuffer(CalibInvalidPixelFile, (void *)CalibInvalidPixelStr.c_str(), CalibInvalidPixelStr.size());
  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();
  deleteFile(ConfigFile);
  return RetVal;
}
