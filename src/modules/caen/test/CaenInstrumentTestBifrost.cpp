// Copyright (C) 2020-2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <caen/CaenInstrument.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>

using namespace Caen;


std::string ConfigFile{"deleteme_bifrost_instr_config.json"};
std::string ConfigStr = R"(
  {
    "Detector": "BIFROST",
    "MaxRing": 4,
    "StrawResolution": 300,
    "XPixels": 900,
    "YPixels": 15
  }
)";


class CaenInstrumentTestBifrost : public TestBase {
protected:
  struct Counters counters;
  CaenSettings ModuleSettings;

  void SetUp() override { ModuleSettings.ConfigFile = ConfigFile; }
  void TearDown() override {}
};

/** Test cases below */
TEST_F(CaenInstrumentTestBifrost, Constructor) {
  CaenInstrument Caen(counters, ModuleSettings);
}


int main(int argc, char **argv) {
  saveBuffer(ConfigFile, (void *)ConfigStr.c_str(), ConfigStr.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(ConfigFile);
  return RetVal;
}
