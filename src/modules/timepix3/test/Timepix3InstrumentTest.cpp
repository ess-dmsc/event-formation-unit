// Copyright (C) 2023-2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include "gtest/gtest.h"
#include <common/kafka/EV44Serializer.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <geometry/Config.h>
#include <logical_geometry/ESSGeometry.h>
#include <memory>
#include <timepix3/Timepix3Instrument.h>

using namespace Timepix3;

std::string ConfigFile{"deleteme_instr_config.json"};
std::string ConfigStr = R"(
  {
    "Detector": "timepix3",
    "ParallelThreads": 1,
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
    "ParallelThreads": 1,
    "XResolution": 256,
    "YResolution": 256
  }
)";

std::string NoDetectorConfigFile{"deleteme_no_detector_config.json"};
std::string NoDetectorConfigStr = R"(
  {
    "ParallelThreads": 1,
    "XResolution": 256,
    "YResolution": 256
  }
)";

std::string NoXResConfigFile{"deleteme_no_xres_config.json"};
std::string NoXResConfigStr = R"(
  {
    "Detector": "timepix3",
    "ParallelThreads": 1,
    "YResolution": 256
  }
)";

std::string BadJsonNoChunkFile{"deleteme_no_chunk_config.json"};
std::string BadJsonNoChunkStr = R"(
  {
    "Detector": "timepix3",
    "XResolution": 256,
    "YResolution": 256
  }
)";

std::vector<uint8_t> SingleGoodReadout{// Single readout
                                       0x91, 0xc6, 0x30, 0x80,
                                       0x8b, 0xa8, 0x3a, 0xbf};

class Timepix3InstrumentTest : public TestBase {
protected:
  struct Counters counters {};

  EV44Serializer serializer;

  Timepix3InstrumentTest() : serializer(115000, "timepix3") {}

  void TearDown() override {}
};

// Test cases below
TEST_F(Timepix3InstrumentTest, Constructor) {
  Timepix3Instrument Timepix3(counters, Config(ConfigFile), serializer);
}

TEST_F(Timepix3InstrumentTest, BadJsonSettings) {
  EXPECT_ANY_THROW(Timepix3Instrument Timepix3(
      counters, Config(BadJsonConfigFile), serializer));
}

TEST_F(Timepix3InstrumentTest, BadNameSettings) {
  EXPECT_ANY_THROW(Timepix3Instrument Timepix3(
      counters, Config(BadNameConfigFile), serializer));
}

TEST_F(Timepix3InstrumentTest, BadJsonNoDetectorSettings) {
  EXPECT_ANY_THROW(Timepix3Instrument Timepix3(
      counters, Config(NoDetectorConfigFile), serializer));
}

TEST_F(Timepix3InstrumentTest, BadJsonNoXResSettings) {
  EXPECT_ANY_THROW(Timepix3Instrument Timepix3(
      counters, Config(NoXResConfigFile), serializer));
}

TEST_F(Timepix3InstrumentTest, BadJsonNoChunkSettings) {
  EXPECT_ANY_THROW(Timepix3Instrument Timepix3(
      counters, Config(BadJsonNoChunkFile), serializer));
}

/// \todo: review this test. What is the main goal.
TEST_F(Timepix3InstrumentTest, SingleGoodReadout) {
  Timepix3Instrument Timepix3(counters, Config(ConfigFile), serializer);
  auto Res = Timepix3.timepix3Parser.parse((char *)SingleGoodReadout.data(),
                                           SingleGoodReadout.size());
  EXPECT_EQ(Res, 1);
  EXPECT_EQ(counters.PixelReadouts, 1);
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
  saveBuffer(BadJsonNoChunkFile, (void *)BadJsonNoChunkStr.c_str(),
             BadJsonNoChunkStr.size());
  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(ConfigFile);
  deleteFile(BadJsonConfigFile);
  deleteFile(BadNameConfigFile);
  deleteFile(NoDetectorConfigFile);
  deleteFile(NoXResConfigFile);
  deleteFile(BadJsonNoChunkFile);
  return RetVal;
}
