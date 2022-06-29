// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <bifrost/BifrostInstrument.h>
#include <common/testutils/SaveBuffer.h>
#include <string.h>
#include <common/testutils/TestBase.h>

using namespace Bifrost;

std::string ConfigFile{"deleteme_bifrost_instr_config.json"};
std::string ConfigStr = R"(
  {
    "Detector" : "Bifrost",

    "MaxPulseTimeNS" : 357142855
  }
)";

class BifrostInstrumentTest : public TestBase {
protected:
  struct Counters counters;
  BifrostSettings ModuleSettings;

  void SetUp() override {
    //ModuleSettings.ConfigFile = ConfigFile;
    counters = {};
  }
  void TearDown() override {}
};

/// Test cases below
TEST_F(BifrostInstrumentTest, Constructor) {
  BifrostInstrument Dream(counters, ModuleSettings);
  ASSERT_EQ(Bifrost.counters.RxPackets, 0);
  ASSERT_EQ(Bifrost.counters.Readouts, 0);
}

// TEST_F(DreamInstrumentTest, CalcPixel) {
//   DreamInstrument Dream(counters, ModuleSettings);
//   /// \todo this is not in agreement with Irina
//   ASSERT_EQ(Dream.calcPixel(22, 3,  3,  1, 15, 15), 329'728);
// }

int main(int argc, char **argv) {
  saveBuffer(ConfigFile, (void *)ConfigStr.c_str(), ConfigStr.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(ConfigFile);
  return RetVal;
}
