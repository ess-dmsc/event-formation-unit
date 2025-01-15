// Copyright (C) 2023-2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/kafka/EV44Serializer.h>
#include <common/testutils/TestBase.h>
#include <dto/TimepixDataTypes.h>
#include <memory>
#include <modules/timepix3/Counters.h>
#include <modules/timepix3/test/TimepixTestHelper.h>
#include <timepix3/readout/DataParser.h>

using namespace Timepix3;
using namespace timepixReadout;
using namespace Observer;

// clang-format off
std::vector<uint8_t> singlePixelReadoutData{
    // Single readout
  0x92, 0x00, 0xd3, 0x4c, // spdr(u16): 146. ftoa(u4): 3, tot(u10): 205
  0xc0, 0x97, 0x01, 0xb1  // toa(u14): 7937, dcol(u8): 16, spix(u7): 12, pix(u1): 1
};

PixelReadout singlePixelReadout{16, 12, 1,7937,
                                                   205, 3, 146};

std::vector<uint8_t> TooShort{
  0x00, 0x01
};

std::vector<uint8_t> tdc1RisingReadoutData{
  // TDC1 rising readout
  0xe0, 0xff, 0xff, 0xff, // timestamp(u35): 34359738367, stamp(u5): 15
  0xff, 0xff, 0xff, 0x6f  // header(u4): 6, type(u4): 15, triggercounter(u12): 4095
};

TDCReadout tdc1RisingReadout{15, 4095, 34359738367,
                                                  15};

std::vector<uint8_t> tdc1FallingReadoutData{
     // TDC1 falling readout
  0xe0, 0xff, 0xff, 0xff, // timestamp(u35): 34359738367, stamp(u5): 15
  0xff, 0xff, 0xff, 0x6a  // header(u4): 6, type(u4): 10, triggercounter(u12): 4095
};

TDCReadout tdc1FallingReadout{10, 4095, 34359738367,
                                                  15};

std::vector<uint8_t> tdc2RisingReadoutData{
    // TDC2 rising readout
  0xe0, 0xff, 0xff, 0xff, // timestamp(u35): 34359738367, stamp(u5): 15
  0xff, 0xff, 0xff, 0x6e  // header(u4): 6, type(u4): 14, triggercounter(u12): 4095
};

TDCReadout tdc2RisingReadout{14, 4095, 34359738367,
                                                  15};

std::vector<uint8_t> tdc2FallingReadoutData{
     // TDC2 falling
  0xe0, 0xff, 0xff, 0xff, // timestamp(u35): 34359738367, stamp(u5): 15
  0xff, 0xff, 0xff, 0x6b  // header(u4): 6, type(u4): 11, triggercounter(u12): 4095
};

TDCReadout tdc2FallingReadout{11, 4095, 34359738367,
                                                  15};

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
  0xe0, 0xff, 0xff, 0xff, // timestamp(u35): 34359738367, stamp(u5): 15
  0xff, 0xff, 0xff, 0x6f,  // header(u4): 6, type(u4): 15, triggercounter(u12): 4095
  // Single pixel readout
  0x92, 0x00, 0xd3, 0x4c, // spdr(u16): 146. ftoa(u4): 3, tot(u10): 205
  0xc0, 0x97, 0x01, 0xb1  // toa(u14): 7937, dcol(u8): 16, spix(u7): 12, pix(u1): 1
};
// clang-format on

class TDCReadoutHandler : public MockupDataEventReceiver<TDCReadout> {};
class EVRReadoutHandler : public MockupDataEventReceiver<EVRReadout> {};
class PixelReadoutHandler : public MockupDataEventReceiver<PixelReadout> {};

class Timepix3ParserTest : public TestBase {
protected:
  Counters counters;
  DataParser timepix3Parser{counters};
  TDCReadoutHandler tdcTestHandler;
  EVRReadoutHandler evrTestHandler;
  PixelReadoutHandler pixelTestHandler;

  EV44Serializer serializer{115000, "timepix3"};

  void SetUp() override {
    new (&counters) Counters();
    new (&timepix3Parser) DataParser(counters);

    timepix3Parser.DataEventObservable<TDCReadout>::subscribe(&tdcTestHandler);
    timepix3Parser.DataEventObservable<EVRReadout>::subscribe(&evrTestHandler);
    timepix3Parser.DataEventObservable<PixelReadout>::subscribe(
        &pixelTestHandler);
  }

  void TearDown() override {}
};

// Test cases below

TEST_F(Timepix3ParserTest, SinglePixelReadout) {
  pixelTestHandler.setData(singlePixelReadout);
  auto Res = timepix3Parser.parseTPX((char *)singlePixelReadoutData.data(),
                                  singlePixelReadoutData.size());
  EXPECT_EQ(Res, 1);
  EXPECT_EQ(counters.PixelReadouts, 1);
}

TEST_F(Timepix3ParserTest, TDC1RisingReadouts) {

  tdcTestHandler.setData(tdc1RisingReadout);

  auto Res = timepix3Parser.parseTPX((char *)tdc1RisingReadoutData.data(),
                                  tdc1RisingReadoutData.size());

  EXPECT_EQ(Res, 1);
  EXPECT_EQ(counters.TDC1RisingReadouts, 1);
  EXPECT_EQ(counters.TDC1FallingReadouts, 0);
  EXPECT_EQ(counters.TDC2RisingReadouts, 0);
  EXPECT_EQ(counters.TDC2FallingReadouts, 0);
}

TEST_F(Timepix3ParserTest, TDC1FallingReadouts) {

  tdcTestHandler.setData(tdc1FallingReadout);

  auto Res = timepix3Parser.parseTPX((char *)tdc1FallingReadoutData.data(),
                                  tdc1FallingReadoutData.size());

  EXPECT_EQ(Res, 1);
  EXPECT_EQ(counters.TDC1FallingReadouts, 1);
  EXPECT_EQ(counters.TDC1RisingReadouts, 0);
  EXPECT_EQ(counters.TDC2RisingReadouts, 0);
  EXPECT_EQ(counters.TDC2FallingReadouts, 0);
}

TEST_F(Timepix3ParserTest, TDC2RisingReadouts) {

  tdcTestHandler.setData(tdc2RisingReadout);

  auto Res = timepix3Parser.parseTPX((char *)tdc2RisingReadoutData.data(),
                                  tdc2RisingReadoutData.size());

  EXPECT_EQ(Res, 1);
  EXPECT_EQ(counters.TDC2RisingReadouts, 1);
  EXPECT_EQ(counters.TDC2FallingReadouts, 0);
  EXPECT_EQ(counters.TDC1RisingReadouts, 0);
  EXPECT_EQ(counters.TDC1FallingReadouts, 0);
}

TEST_F(Timepix3ParserTest, TDC2FallingReadouts) {

  tdcTestHandler.setData(tdc2FallingReadout);

  auto Res = timepix3Parser.parseTPX((char *)tdc2FallingReadoutData.data(),
                                  tdc2FallingReadoutData.size());

  EXPECT_EQ(Res, 1);
  EXPECT_EQ(counters.TDC2FallingReadouts, 1);
  EXPECT_EQ(counters.TDC2RisingReadouts, 0);
  EXPECT_EQ(counters.TDC1RisingReadouts, 0);
  EXPECT_EQ(counters.TDC1FallingReadouts, 0);
}

TEST_F(Timepix3ParserTest, TooShort) {
  auto Res = timepix3Parser.parseTPX((char *)TooShort.data(), TooShort.size());
  EXPECT_EQ(Res, 0);
}

TEST_F(Timepix3ParserTest, SingleEVRReadout) {
  evrTestHandler.setData(singleEVRReadout);
  auto Res = timepix3Parser.parseTPX((char *)SingleEVRReadoutData.data(),
                                  SingleEVRReadoutData.size());
  EXPECT_EQ(Res, 1);
  EXPECT_EQ(counters.EVRReadoutCounter, 1);
}

TEST_F(Timepix3ParserTest, TDCAndPixelReadout) {
  tdcTestHandler.setData(tdc1RisingReadout);
  pixelTestHandler.setData(singlePixelReadout);

  auto Res = timepix3Parser.parseTPX((char *)TDCAndPixelReadout.data(),
                                  TDCAndPixelReadout.size());
  EXPECT_EQ(Res, 2);
  EXPECT_EQ(counters.TDC1RisingReadouts, 1);
  EXPECT_EQ(counters.TDCReadoutCounter, 1);
  EXPECT_EQ(counters.PixelReadouts, 1);
}

int main(int argc, char **argv) {

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  return RetVal;
}