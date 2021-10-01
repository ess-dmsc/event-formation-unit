// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <dream/DreamInstrument.h>
#include <common/testutils/SaveBuffer.h>
#include <string.h>
#include <common/testutils/TestBase.h>

using namespace Dream;

std::string ConfigFile{"deleteme_dream_instr_config.json"};
std::string ConfigStr = R"(
  {
    "Detector" : "Dream",

    "MaxPulseTimeNS" : 357142855
  }
)";

class DreamInstrumentTest : public TestBase {
protected:
  struct Counters counters;
  DreamSettings ModuleSettings;

  void SetUp() override {
    ModuleSettings.ConfigFile = ConfigFile;
    counters = {};
  }
  void TearDown() override {}
};

/// Test cases below
TEST_F(DreamInstrumentTest, Constructor) {
  DreamInstrument Dream(counters, ModuleSettings);
  ASSERT_EQ(Dream.counters.RxPackets, 0);
  ASSERT_EQ(Dream.counters.Readouts, 0);
}

TEST_F(DreamInstrumentTest, CalcPixel) {
  DreamInstrument Dream(counters, ModuleSettings);
  /// \todo this is not in agreement with Irina
  ASSERT_EQ(Dream.calcPixel(1, 6, 1, 16, 10, 2), 1);
}

int main(int argc, char **argv) {
  saveBuffer(ConfigFile, (void *)ConfigStr.c_str(), ConfigStr.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(ConfigFile);
  return RetVal;
}
