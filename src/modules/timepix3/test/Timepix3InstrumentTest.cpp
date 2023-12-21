// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <logical_geometry/ESSGeometry.h>
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

std::string BadJsonConfigFile{"deleteme_bad_instr_config.json"};
std::string BadJsonConfigStr = R"(
  {
    invalidjson...
  }
)";

std::string BadNameConfigFile{"deleteme_bad_name_config.json"};
std::string BadNameConfigStr = R"(
  {
    "Detector": "invalid name",
    "XResolution": 256,
    "YResolution": 256
  }
)";

std::string NoDetectorConfigFile{"deleteme_no_detector_config.json"};
std::string NoDetectorConfigStr = R"(
  {
    "XResolution": 256,
    "YResolution": 256
  }
)";

std::string NoXResConfigFile{"deleteme_no_xres_config.json"};
std::string NoXResConfigStr = R"(
  {
    "Detector": "timepix3",
    "YResolution": 256
  }
)";

std::vector<uint8_t> SingleGoodReadout{// Single readout
                                       0x91, 0xc6, 0x30, 0x80,
                                       0x8b, 0xa8, 0x3a, 0xbf};

class Timepix3InstrumentTest : public TestBase {
protected:
  struct Counters counters;
  BaseSettings Settings;

  Timepix3Instrument *timepix3;

  void SetUp() override {
    Settings.ConfigFile = ConfigFile;
    counters = {};
    timepix3 = new Timepix3Instrument(counters, Settings);
    timepix3->Serializer = new EV44Serializer(115000, "timepix3");
  }
  void TearDown() override {}
};

// Test cases below
TEST_F(Timepix3InstrumentTest, Constructor) {
  Timepix3Instrument Timepix3(counters, Settings);
}

TEST_F(Timepix3InstrumentTest, BadJsonSettings) {
  Settings.ConfigFile = BadJsonConfigFile;
  EXPECT_ANY_THROW(Timepix3Instrument Timepix3(counters, Settings));
}

TEST_F(Timepix3InstrumentTest, BadNameSettings) {
  Settings.ConfigFile = BadNameConfigFile;
  EXPECT_ANY_THROW(Timepix3Instrument Timepix3(counters, Settings));
}

TEST_F(Timepix3InstrumentTest, BadJsonNoDetectorSettings) {
  Settings.ConfigFile = NoDetectorConfigFile;
  EXPECT_ANY_THROW(Timepix3Instrument Timepix3(counters, Settings));
}

TEST_F(Timepix3InstrumentTest, BadJsonNoXResSettings) {
  Settings.ConfigFile = NoXResConfigFile;
  EXPECT_ANY_THROW(Timepix3Instrument Timepix3(counters, Settings));
}

TEST_F(Timepix3InstrumentTest, SingleGoodReadout) {
  auto Res = timepix3->timepix3Parser.parse((char *)SingleGoodReadout.data(),
  SingleGoodReadout.size());
  ASSERT_EQ(Res, 1);
  ASSERT_EQ(counters.PixelReadouts, 1);
  
  timepix3->processReadouts();
  ASSERT_EQ(counters.Events, 1);
}

int main(int argc, char **argv) {
  saveBuffer(ConfigFile, (void *)ConfigStr.c_str(), ConfigStr.size());
  saveBuffer(BadJsonConfigFile, (void *)BadJsonConfigStr.c_str(),
             BadJsonConfigStr.size());
  saveBuffer(BadNameConfigFile, (void *)BadNameConfigStr.c_str(),
             BadNameConfigStr.size());
  saveBuffer(NoDetectorConfigFile, (void *)NoDetectorConfigStr.c_str(),
             NoDetectorConfigStr.size());
  saveBuffer(NoXResConfigFile, (void *)NoXResConfigStr.c_str(),
             NoXResConfigStr.size());
  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(ConfigFile);
  deleteFile(BadJsonConfigFile);
  deleteFile(BadNameConfigFile);
  deleteFile(NoDetectorConfigFile);
  deleteFile(NoXResConfigFile);
  return RetVal;
}
