// Copyright (C) 2020-2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <timepix3/Timepix3Instrument.h>

using namespace Timepix3;

std::string ConfigFile{"deleteme_instr_config.json"};
std::string ConfigStr = R"(
  {
    "Detector": "timepix3",
    "XResolution": 128,
    "YResolution": 128
  }
)";

class Timepix3InstrumentTest : public TestBase {
protected:
  struct Counters counters;
  BaseSettings Settings;

  void SetUp() override { Settings.ConfigFile = ConfigFile; }
  void TearDown() override {}
};

// Test cases below
TEST_F(Timepix3InstrumentTest, Constructor) {
  Timepix3Instrument Timepix3(counters, Settings);
}

int main(int argc, char **argv) {
  saveBuffer(ConfigFile, (void *)ConfigStr.c_str(), ConfigStr.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(ConfigFile);
  return RetVal;
}
