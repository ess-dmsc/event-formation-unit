// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include "common/dataflow/DataObserverTemplate.h"
#include "readout/DataEventTypes.h"
#include "readout/TimingEventHandler.h"
#include "gtest/gtest-death-test.h"
#include "gtest/gtest.h"
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <cstdint>
#include <cstdio>
#include <timepix3/readout/DataParser.h>

using namespace Timepix3;

// clang-format off
std::vector<uint8_t> SinglePixelReadout{
    // Single readout
    0x91, 0xc6, 0x30, 0x80,
    0x8b, 0xa8, 0x3a, 0xbf
};

std::vector<uint8_t> TooShort{
    0x00, 0x01
};

std::vector<uint8_t> TDC1RisingReadout{
    // TDC1 rising readout
    0xc0, 0x42, 0x9f, 0xdd,
    0xa4, 0x7e, 0x8b, 0x6f
};

std::vector<uint8_t> TDC1FallingReadout{
     // TDC1 falling readout
    0xc0, 0x42, 0x9f, 0xdd,
    0xa4, 0x7e, 0x8b, 0x6a
};

std::vector<uint8_t> TDC2RisingReadout{
    // TDC2 rising readout
    0xc0, 0x42, 0x9f, 0xdd,
    0xa4, 0x7e, 0x8b, 0x6e
};

std::vector<uint8_t> TDC2FallingReadout{
     // TDC2 falling
    0xc0, 0x42, 0x9f, 0xdd,
    0xa4, 0x7e, 0x8b, 0x6b
};

std::vector<uint8_t> SingleEVRReadout{
  0x01, 0x45, 0x53, 0x53,
  0x2a, 0xc0, 0x08, 0x00,
  0xcc, 0xa1, 0x3f, 0x64,
  0xdd, 0xc9, 0x9a, 0x3b,
  0xcb, 0xa1, 0x3f, 0x64,
  0xdd, 0xc9, 0x9a, 0x3b
};

std::vector<uint8_t> TDCAndPixelReadout{
  // Single TDC readout
  0xc0, 0x42, 0x9f, 0xdd,
  0xa4, 0x7e, 0x8b, 0x6f,
  // Single pixel readout
  0x91, 0xc6, 0x30, 0x80,
  0x8b, 0xa8, 0x3a, 0xbf
};
// clang-format on


class Timepix3ParserTest : public TestBase {
protected:
  struct Counters counters;
  TimingEventHandler testEventHandler;
  DataParser Timepix3Parser{counters, testEventHandler};

  void SetUp() override {counters = {}; }
  void TearDown() override {}
};

// Test cases below

TEST_F(Timepix3ParserTest, SinglePixelReadout) {
  auto Res = Timepix3Parser.parse((char *)SinglePixelReadout.data(),
                                   SinglePixelReadout.size());
  EXPECT_EQ(Res, 1);
  EXPECT_EQ(counters.PixelReadouts, 1);
}

TEST_F(Timepix3ParserTest, TDCReadouts) {
  auto Res = Timepix3Parser.parse((char *)TDC1RisingReadout.data(),
                                   TDC1RisingReadout.size());
  EXPECT_EQ(Res, 1);
  EXPECT_EQ(counters.TDCReadouts, 1);
  EXPECT_EQ(counters.TDC1RisingReadouts, 1);
  EXPECT_EQ(counters.TDC1FallingReadouts, 0);
  EXPECT_EQ(counters.TDC2RisingReadouts, 0);
  EXPECT_EQ(counters.TDC2FallingReadouts, 0);

  EXPECT_TRUE(testEventHandler.getLastTdcEvent() != nullptr);
  EXPECT_TRUE(testEventHandler.getLastTdcEvent()->Type == 15);
  EXPECT_TRUE(testEventHandler.getLastTdcEvent()->TriggerCounter == 2231);
  EXPECT_TRUE(testEventHandler.getLastTdcEvent()->Timestamp == 31447764897);
  EXPECT_TRUE(testEventHandler.getLastTdcEvent()->Stamp == 6);

  Res = Timepix3Parser.parse((char *)TDC1FallingReadout.data(),
                                   TDC1FallingReadout.size());
  EXPECT_EQ(Res, 1);
  EXPECT_EQ(counters.TDCReadouts, 2);
  EXPECT_EQ(counters.TDC1RisingReadouts, 1);
  EXPECT_EQ(counters.TDC1FallingReadouts, 1);
  EXPECT_EQ(counters.TDC2RisingReadouts, 0);
  EXPECT_EQ(counters.TDC2FallingReadouts, 0);

  EXPECT_TRUE(testEventHandler.getLastTdcEvent() != nullptr);
  EXPECT_TRUE(testEventHandler.getLastTdcEvent()->Type == 10);
  EXPECT_TRUE(testEventHandler.getLastTdcEvent()->TriggerCounter == 2231);
  EXPECT_TRUE(testEventHandler.getLastTdcEvent()->Timestamp == 31447764897);
  EXPECT_TRUE(testEventHandler.getLastTdcEvent()->Stamp == 6);

  Res = Timepix3Parser.parse((char *)TDC2RisingReadout.data(),
                                   TDC2RisingReadout.size());
  EXPECT_EQ(Res, 1);
  EXPECT_EQ(counters.TDCReadouts, 3);
  EXPECT_EQ(counters.TDC1RisingReadouts, 1);
  EXPECT_EQ(counters.TDC1FallingReadouts, 1);
  EXPECT_EQ(counters.TDC2RisingReadouts, 1);
  EXPECT_EQ(counters.TDC2FallingReadouts, 0);

  EXPECT_TRUE(testEventHandler.getLastTdcEvent() != nullptr);
  EXPECT_TRUE(testEventHandler.getLastTdcEvent()->Type == 14);
  EXPECT_TRUE(testEventHandler.getLastTdcEvent()->TriggerCounter == 2231);
  EXPECT_TRUE(testEventHandler.getLastTdcEvent()->Timestamp == 31447764897);
  EXPECT_TRUE(testEventHandler.getLastTdcEvent()->Stamp == 6);

  Res = Timepix3Parser.parse((char *)TDC2FallingReadout.data(),
                                   TDC2FallingReadout.size());
  EXPECT_EQ(Res, 1);
  EXPECT_EQ(counters.TDCReadouts, 4);
  EXPECT_EQ(counters.TDC1RisingReadouts, 1);
  EXPECT_EQ(counters.TDC1FallingReadouts, 1);
  EXPECT_EQ(counters.TDC2RisingReadouts, 1);
  EXPECT_EQ(counters.TDC2FallingReadouts, 1);

  EXPECT_TRUE(testEventHandler.getLastTdcEvent() != nullptr);
  EXPECT_TRUE(testEventHandler.getLastTdcEvent()->Type == 11);
  EXPECT_TRUE(testEventHandler.getLastTdcEvent()->TriggerCounter == 2231);
  EXPECT_TRUE(testEventHandler.getLastTdcEvent()->Timestamp == 31447764897);
  EXPECT_TRUE(testEventHandler.getLastTdcEvent()->Stamp == 6);
}

TEST_F(Timepix3ParserTest, TooShort) {
  auto Res = Timepix3Parser.parse((char *)TooShort.data(), TooShort.size());
  EXPECT_EQ(Res, 0);
}

TEST_F(Timepix3ParserTest, SingleEVRReadout) {
  auto Res = Timepix3Parser.parse((char *)SingleEVRReadout.data(),
                                   SingleEVRReadout.size());
  EXPECT_EQ(Res, 1);
  EXPECT_EQ(counters.EVRTimestampReadouts, 1);
}

TEST_F(Timepix3ParserTest, TDCAndPixelReadout) {
  auto Res = Timepix3Parser.parse((char *)TDCAndPixelReadout.data(),
                                   TDCAndPixelReadout.size());
  EXPECT_EQ(Res, 2);
  EXPECT_EQ(counters.TDCReadouts, 1);
  EXPECT_EQ(counters.PixelReadouts, 1);
  EXPECT_EQ(counters.PixelReadoutFromBeforeTDC, 1);

  EXPECT_TRUE(testEventHandler.getLastTdcEvent() != nullptr);
  EXPECT_TRUE(testEventHandler.getLastTdcEvent()->Type == 10);
  EXPECT_TRUE(testEventHandler.getLastTdcEvent()->TriggerCounter == 2231);
  EXPECT_TRUE(testEventHandler.getLastTdcEvent()->Timestamp == 31447764897);
  EXPECT_TRUE(testEventHandler.getLastTdcEvent()->Stamp == 6);
  
}

int main(int argc, char **argv) {

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  return RetVal;
}
