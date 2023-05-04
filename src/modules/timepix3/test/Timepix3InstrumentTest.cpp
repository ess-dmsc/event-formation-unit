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
    "XResolution": 256,
    "YResolution": 256
  }
)";

std::vector<uint8_t> SingleGoodReadout{
    // Single readout
    0x91, 0xc6, 0x30, 0x80,
    0x8b, 0xa8, 0x3a, 0xbf
};

class Timepix3InstrumentTest : public TestBase {
protected:
  struct Counters counters;
  BaseSettings Settings;

  Timepix3Instrument *timepix3;

  void SetUp() override {
     Settings.ConfigFile = ConfigFile;
     timepix3 = new Timepix3Instrument(counters, Settings);
   }
  void TearDown() override {}
};

// Test cases below
TEST_F(Timepix3InstrumentTest, Constructor) {
  Timepix3Instrument Timepix3(counters, Settings);
}

TEST_F(Timepix3InstrumentTest, SingleGoodReadout) {
  auto Res = timepix3->Timepix3Parser.parse((char *)SingleGoodReadout.data(), SingleGoodReadout.size());
  ASSERT_EQ(Res, 1);
  ASSERT_EQ(counters.PixelReadouts, 1);

  timepix3->processReadouts();

  // ASSERT_EQ(counters.Events, 1);
}

int main(int argc, char **argv) {
  saveBuffer(ConfigFile, (void *)ConfigStr.c_str(), ConfigStr.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(ConfigFile);
  return RetVal;
}
