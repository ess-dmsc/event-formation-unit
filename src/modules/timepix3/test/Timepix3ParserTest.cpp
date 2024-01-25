// Copyright (C) 2023-2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include "Counters.h"
#include "TimepixTestHelper.h"
#include "common/kafka/EV44Serializer.h"
#include "readout/TimepixDataTypes.h"
#include <common/testutils/TestBase.h>
#include <memory>
#include <timepix3/readout/DataParser.h>

using namespace Timepix3;
using namespace timepixReadout;
using namespace Observer;

// clang-format off
std::vector<uint8_t> singlePixelReadoutData{
    // Single readout
    0x91, 0xc6, 0x30, 0x80,
    0x8b, 0xa8, 0x3a, 0xbf
};

PixelReadout singlePixelReadout{15, 2231, 30,
                                                  6, 45, 454, 33};

std::vector<uint8_t> TooShort{
    0x00, 0x01
};

std::vector<uint8_t> tdc1RisingReadoutData{
     // TDC1 rising readout
    0xc0, 0x42, 0x9f, 0xdd,
    0xa4, 0x7e, 0x8b, 0x6f
};

TDCReadout tdc1RisingReadout{15, 2231, 31447764897,
                                                  6};

std::vector<uint8_t> tdc1FallingReadoutData{
     // TDC1 falling readout
    0xc0, 0x42, 0x9f, 0xdd,
    0xa4, 0x7e, 0x8b, 0x6a
};

TDCReadout tdc1FallingReadout{10, 2231, 31447764897,
                                                  6};

std::vector<uint8_t> tdc2RisingReadoutData{
    // TDC2 rising readout
    0xc0, 0x42, 0x9f, 0xdd,
    0xa4, 0x7e, 0x8b, 0x6e
};

TDCReadout tdc2RisingReadout{14, 2231, 31447764897,
                                                  6};

std::vector<uint8_t> tdc2FallingReadoutData{
     // TDC2 falling
    0xc0, 0x42, 0x9f, 0xdd,
    0xa4, 0x7e, 0x8b, 0x6b
};

TDCReadout tdc2FallingReadout{11, 2231, 31447764897,
                                                  6};

std::vector<uint8_t> SingleEVRReadoutData{
  0x01, 0x45, 0x53, 0x53,
  0x2a, 0xc0, 0x08, 0x00,
  0xcc, 0xa1, 0x3f, 0x64,
  0xdd, 0xc9, 0x9a, 0x3b,
  0xcb, 0xa1, 0x3f, 0x64,
  0xdd, 0xc9, 0x9a, 0x3b
};

EVRReadout singleEVRReadout{1,
 69,
  21331,
   573482,
    static_cast<uint32_t>(1681891788),
     static_cast<uint32_t>(999999965),
      1681891787,
       999999965};

std::vector<uint8_t> TDCAndPixelReadout{
  // Single TDC readout
  0xc0, 0x42, 0x9f, 0xdd,
  0xa4, 0x7e, 0x8b, 0x6f,
  // Single pixel readout
  0x91, 0xc6, 0x30, 0x80,
  0x8b, 0xa8, 0x3a, 0xbf
};
// clang-format on

class TDCReadoutHandler : public Timpix3OberverTestHelper<TDCReadout> {};
class EVRReadoutHandler : public Timpix3OberverTestHelper<EVRReadout> {};
class PixelReadoutHandler : public Timpix3OberverTestHelper<PixelReadout> {};

class Timepix3ParserTest : public TestBase {
protected:
  std::unique_ptr<Counters> counters;
  std::unique_ptr<DataParser> timepix3Parser;
  TDCReadoutHandler tdcTestHandler;
  EVRReadoutHandler evrTestHandler;
  PixelReadoutHandler pixelTestHandler;

  EV44Serializer serializer{115000, "timepix3"};

  void SetUp() override {
    counters = std::make_unique<Counters>(1);
    timepix3Parser.reset(new DataParser(*counters));

    timepix3Parser->DataEventObservable<TDCReadout>::subscribe(&tdcTestHandler);
    timepix3Parser->DataEventObservable<EVRReadout>::subscribe(&evrTestHandler);
  }

  void TearDown() override {}
};

// Test cases below

TEST_F(Timepix3ParserTest, SinglePixelReadout) {
  pixelTestHandler.setData(singlePixelReadout);
  auto Res = timepix3Parser->parse((char *)singlePixelReadoutData.data(),
                                   singlePixelReadoutData.size());
  EXPECT_EQ(Res, 1);
  EXPECT_EQ(counters->PixelReadouts, 1);
}

TEST_F(Timepix3ParserTest, TDC1RisingReadouts) {

  tdcTestHandler.setData(tdc1RisingReadout);

  auto Res = timepix3Parser->parse((char *)tdc1RisingReadoutData.data(),
                                   tdc1RisingReadoutData.size());

  EXPECT_EQ(Res, 1);
  EXPECT_EQ(counters->TDC1RisingReadouts, 1);
  EXPECT_EQ(counters->TDC1FallingReadouts, 0);
  EXPECT_EQ(counters->TDC2RisingReadouts, 0);
  EXPECT_EQ(counters->TDC2FallingReadouts, 0);
}

TEST_F(Timepix3ParserTest, TDC1FallingReadouts) {

  tdcTestHandler.setData(tdc1FallingReadout);

  auto Res = timepix3Parser->parse((char *)tdc1FallingReadoutData.data(),
                                   tdc1FallingReadoutData.size());

  EXPECT_EQ(Res, 1);
  EXPECT_EQ(counters->TDC1FallingReadouts, 1);
  EXPECT_EQ(counters->TDC1RisingReadouts, 0);
  EXPECT_EQ(counters->TDC2RisingReadouts, 0);
  EXPECT_EQ(counters->TDC2FallingReadouts, 0);
}

TEST_F(Timepix3ParserTest, TDC2RisingReadouts) {

  tdcTestHandler.setData(tdc2RisingReadout);

  auto Res = timepix3Parser->parse((char *)tdc2RisingReadoutData.data(),
                                   tdc2RisingReadoutData.size());

  EXPECT_EQ(Res, 1);
  EXPECT_EQ(counters->TDC2RisingReadouts, 1);
  EXPECT_EQ(counters->TDC2FallingReadouts, 0);
  EXPECT_EQ(counters->TDC1RisingReadouts, 0);
  EXPECT_EQ(counters->TDC1FallingReadouts, 0);
}

TEST_F(Timepix3ParserTest, TDC2FallingReadouts) {

  tdcTestHandler.setData(tdc2FallingReadout);

  auto Res = timepix3Parser->parse((char *)tdc2FallingReadoutData.data(),
                                   tdc2FallingReadoutData.size());

  EXPECT_EQ(Res, 1);
  EXPECT_EQ(counters->TDC2FallingReadouts, 1);
  EXPECT_EQ(counters->TDC2RisingReadouts, 0);
  EXPECT_EQ(counters->TDC1RisingReadouts, 0);
  EXPECT_EQ(counters->TDC1FallingReadouts, 0);
}

TEST_F(Timepix3ParserTest, TooShort) {
  auto Res = timepix3Parser->parse((char *)TooShort.data(), TooShort.size());
  EXPECT_EQ(Res, 0);
}

TEST_F(Timepix3ParserTest, SingleEVRReadout) {
  evrTestHandler.setData(singleEVRReadout);
  auto Res = timepix3Parser->parse((char *)SingleEVRReadoutData.data(),
                                   SingleEVRReadoutData.size());
  EXPECT_EQ(Res, 1);
  EXPECT_EQ(counters->EVRTimeStampReadouts, 1);
}

TEST_F(Timepix3ParserTest, TDCAndPixelReadout) {
  tdcTestHandler.setData(tdc1RisingReadout);
  pixelTestHandler.setData(singlePixelReadout);

  auto Res = timepix3Parser->parse((char *)TDCAndPixelReadout.data(),
                                   TDCAndPixelReadout.size());
  EXPECT_EQ(Res, 2);
  EXPECT_EQ(counters->TDC1RisingReadouts, 1);
  EXPECT_EQ(counters->TDCTimeStampReadout, 1);
  EXPECT_EQ(counters->PixelReadouts, 1);
}

int main(int argc, char **argv) {

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  return RetVal;
}