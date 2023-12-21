// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include "common/utils/UnitConverter.h"
#include "readout/TimingEventHandler.h"
#include "gtest/gtest-death-test.h"
#include "gtest/gtest.h"
#include "gtest/internal/gtest-port.h"
#include <algorithm>
#include <cmath>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <timepix3/readout/DataParser.h>

using namespace Timepix3;
using namespace efutils;

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
  unique_ptr<Counters> counters;
  unique_ptr<DataParser> Timepix3Parser;
  unique_ptr<TimingEventHandler> testEventHandler;
  const uint64_t testTdcTimeStamp =
      TDC_CLOCK_BIN_NS * 31447764897 + TDC_FINE_CLOCK_BIN_NS * 6;

  void SetUp() override {
    counters.reset(new Counters());
    testEventHandler.reset(new TimingEventHandler(*counters));
    Timepix3Parser.reset(new DataParser(*counters, *testEventHandler));
  }
  void TearDown() override {}
};

// Test cases below

TEST_F(Timepix3ParserTest, SinglePixelReadout) {
  auto Res = Timepix3Parser->parse((char *)SinglePixelReadout.data(),
                                   SinglePixelReadout.size());
  EXPECT_EQ(Res, 1);
  EXPECT_EQ(counters->PixelReadouts, 1);
}

TEST_F(Timepix3ParserTest, TDCReadouts) {
  auto Res = Timepix3Parser->parse((char *)TDC1RisingReadout.data(),
                                   TDC1RisingReadout.size());
  EXPECT_EQ(Res, 1);
  EXPECT_EQ(counters->TDCReadouts, 1);
  EXPECT_EQ(counters->TDC1RisingReadouts, 1);
  EXPECT_EQ(counters->TDC1FallingReadouts, 0);
  EXPECT_EQ(counters->TDC2RisingReadouts, 0);
  EXPECT_EQ(counters->TDC2FallingReadouts, 0);

  EXPECT_TRUE(testEventHandler->getLastTdcEvent() != nullptr);
  EXPECT_TRUE(testEventHandler->getLastTdcEvent()->type == 15);
  EXPECT_TRUE(testEventHandler->getLastTdcEvent()->counter == 2231);
  EXPECT_TRUE(testEventHandler->getLastTdcEvent()->tdcTimeStamp ==
              testTdcTimeStamp);
  EXPECT_TRUE(testEventHandler->getLastTdcEvent()->nextTdcStimeStamp ==
              testTdcTimeStamp + testEventHandler->getTDCFrequency());

  Res = Timepix3Parser->parse((char *)TDC1FallingReadout.data(),
                              TDC1FallingReadout.size());
  EXPECT_EQ(Res, 1);
  EXPECT_EQ(counters->TDCReadouts, 2);
  EXPECT_EQ(counters->TDC1RisingReadouts, 1);
  EXPECT_EQ(counters->TDC1FallingReadouts, 1);
  EXPECT_EQ(counters->TDC2RisingReadouts, 0);
  EXPECT_EQ(counters->TDC2FallingReadouts, 0);

  EXPECT_TRUE(testEventHandler->getLastTdcEvent() != nullptr);
  EXPECT_TRUE(testEventHandler->getLastTdcEvent()->type == 10);
  EXPECT_TRUE(testEventHandler->getLastTdcEvent()->counter == 2231);
  EXPECT_TRUE(testEventHandler->getLastTdcEvent()->tdcTimeStamp ==
              testTdcTimeStamp);
  EXPECT_TRUE(testEventHandler->getLastTdcEvent()->nextTdcStimeStamp ==
              testTdcTimeStamp + testEventHandler->getTDCFrequency());

  Res = Timepix3Parser->parse((char *)TDC2RisingReadout.data(),
                              TDC2RisingReadout.size());
  EXPECT_EQ(Res, 1);
  EXPECT_EQ(counters->TDCReadouts, 3);
  EXPECT_EQ(counters->TDC1RisingReadouts, 1);
  EXPECT_EQ(counters->TDC1FallingReadouts, 1);
  EXPECT_EQ(counters->TDC2RisingReadouts, 1);
  EXPECT_EQ(counters->TDC2FallingReadouts, 0);

  EXPECT_TRUE(testEventHandler->getLastTdcEvent() != nullptr);
  EXPECT_TRUE(testEventHandler->getLastTdcEvent()->type == 14);
  EXPECT_TRUE(testEventHandler->getLastTdcEvent()->counter == 2231);
  EXPECT_TRUE(testEventHandler->getLastTdcEvent()->tdcTimeStamp ==
              testTdcTimeStamp);
  EXPECT_TRUE(testEventHandler->getLastTdcEvent()->nextTdcStimeStamp ==
              testTdcTimeStamp + testEventHandler->getTDCFrequency());

  Res = Timepix3Parser->parse((char *)TDC2FallingReadout.data(),
                              TDC2FallingReadout.size());
  EXPECT_EQ(Res, 1);
  EXPECT_EQ(counters->TDCReadouts, 4);
  EXPECT_EQ(counters->TDC1RisingReadouts, 1);
  EXPECT_EQ(counters->TDC1FallingReadouts, 1);
  EXPECT_EQ(counters->TDC2RisingReadouts, 1);
  EXPECT_EQ(counters->TDC2FallingReadouts, 1);

  EXPECT_TRUE(testEventHandler->getLastTdcEvent() != nullptr);
  EXPECT_TRUE(testEventHandler->getLastTdcEvent()->type == 11);
  EXPECT_TRUE(testEventHandler->getLastTdcEvent()->counter == 2231);
  EXPECT_TRUE(testEventHandler->getLastTdcEvent()->tdcTimeStamp ==
              testTdcTimeStamp);
  EXPECT_TRUE(testEventHandler->getLastTdcEvent()->nextTdcStimeStamp ==
              testTdcTimeStamp + testEventHandler->getTDCFrequency());
}

TEST_F(Timepix3ParserTest, TooShort) {
  auto Res = Timepix3Parser->parse((char *)TooShort.data(), TooShort.size());
  EXPECT_EQ(Res, 0);
}

TEST_F(Timepix3ParserTest, SingleEVRReadout) {
  auto Res = Timepix3Parser->parse((char *)SingleEVRReadout.data(),
                                   SingleEVRReadout.size());
  EXPECT_EQ(Res, 1);
  EXPECT_EQ(counters->EVRTimeStampReadouts, 1);
}

TEST_F(Timepix3ParserTest, TDCAndPixelReadout) {
  auto Res = Timepix3Parser->parse((char *)TDCAndPixelReadout.data(),
                                   TDCAndPixelReadout.size());
  EXPECT_EQ(Res, 2);
  EXPECT_EQ(counters->TDCReadouts, 1);
  EXPECT_EQ(counters->PixelReadouts, 1);
  EXPECT_EQ(counters->PixelReadoutFromBeforeTDC, 1);

  EXPECT_TRUE(testEventHandler->getLastTdcEvent() != nullptr);
  EXPECT_TRUE(testEventHandler->getLastTdcEvent()->type == 15);
  EXPECT_TRUE(testEventHandler->getLastTdcEvent()->counter == 2231);
  EXPECT_TRUE(testEventHandler->getLastTdcEvent()->tdcTimeStamp ==
              testTdcTimeStamp);
  EXPECT_TRUE(testEventHandler->getLastTdcEvent()->nextTdcStimeStamp ==
              testTdcTimeStamp + testEventHandler->getTDCFrequency());
}

TEST_F(Timepix3ParserTest, FindEVRandTDCPairs) {
  auto Res = Timepix3Parser->parse((char *)TDC1RisingReadout.data(),
                                   TDC1RisingReadout.size());

  Res += Timepix3Parser->parse((char *)SingleEVRReadout.data(),
                               SingleEVRReadout.size());
  EXPECT_EQ(Res, 2);
  EXPECT_EQ(counters->TDCReadouts, 1);
  EXPECT_EQ(counters->EVRTimeStampReadouts, 1);
  EXPECT_EQ(counters->PixelReadouts, 0);
  EXPECT_EQ(counters->FoundEVRandTDCPairs, 1);
  EXPECT_EQ(counters->MissTDCPair, 0);
  EXPECT_EQ(counters->MissEVRPair, 0);

  EXPECT_TRUE(testEventHandler->getLastTdcEvent() != nullptr);
  EXPECT_TRUE(testEventHandler->getLastTDCPair() != nullptr);
  EXPECT_EQ(testEventHandler->getLastTDCPair(),
            testEventHandler->getLastTdcEvent());
}

int main(int argc, char **argv) {

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  return RetVal;
}