// Copyright (C) 2017-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for ReadoutParser
///
//===----------------------------------------------------------------------===//

#include <readout/common/ReadoutParser.h>
#include <readout/vmm3/VMM3Parser.h>
#include <readout/vmm3/test/VMM3ParserTestData.h>
#include <test/TestBase.h>


class VMM3ParserTest : public TestBase {
protected:
  ReadoutParser::PacketDataV0 PacketData;
  VMM3Parser Parser;
  void SetUp() override {
    PacketData.HeaderPtr = nullptr;
    PacketData.DataPtr = nullptr;
    PacketData.DataLength = 0;
    PacketData.Time.setReference(0,0);
    PacketData.Time.setPrevReference(0,0);
  }
  void TearDown() override {}

  void makeHeader(std::vector<uint8_t> & testdata) {
    PacketData.DataPtr = (char *)&testdata[0];
    PacketData.DataLength = testdata.size();
  }
};


TEST_F(VMM3ParserTest, Constructor) {
  ASSERT_EQ(Parser.Result.size(), 0);
}

// nullptr as buffer
TEST_F(VMM3ParserTest, ErrorBufferPtr) {
  PacketData.DataPtr = 0;
  PacketData.DataLength = 100;
  auto Res = Parser.parse(PacketData);
  ASSERT_EQ(Res, 0);
  ASSERT_EQ(Parser.Stats.ErrorSize, 1);
  ASSERT_EQ(Parser.Stats.Readouts, 0);
}

// invalid data size
TEST_F(VMM3ParserTest, ErrorAPISize) {
  makeHeader(VMMData1);
  PacketData.DataLength = 19;

  auto Res = Parser.parse(PacketData);
  ASSERT_EQ(Res, 0);
  ASSERT_EQ(Parser.Stats.ErrorSize, 1);
  ASSERT_EQ(Parser.Stats.Readouts, 0);
}

// Invalid RingID
TEST_F(VMM3ParserTest, ErrorRing) {
  makeHeader(VMMRingError);
  auto Res = Parser.parse(PacketData);
  ASSERT_EQ(Res, 1);
  ASSERT_EQ(Parser.Stats.Readouts, 2);
  ASSERT_EQ(Parser.Stats.ErrorRing, 1);
}

// Invalid FENId
TEST_F(VMM3ParserTest, ErrorFEN) {
  makeHeader(VMMFENError);
  auto Res = Parser.parse(PacketData);
  ASSERT_EQ(Res, 1);
  ASSERT_EQ(Parser.Stats.Readouts, 2);
  ASSERT_EQ(Parser.Stats.ErrorFEN, 1);
}

// Invalid data length - so far always 20 bytes.
TEST_F(VMM3ParserTest, ErrorDataLength) {
  makeHeader(VMMDataLengthError);
  auto Res = Parser.parse(PacketData);
  ASSERT_EQ(Res, 1);
  ASSERT_EQ(Parser.Stats.Readouts, 2);
  ASSERT_EQ(Parser.Stats.ErrorDataLength, 1);
}

// Testing valid and invalid TimeLO ranges
TEST_F(VMM3ParserTest, ErrorTimeLow) {
  makeHeader(VMMTimeLowError);
  auto Res = Parser.parse(PacketData);
  ASSERT_EQ(Res, 2);
  ASSERT_EQ(Parser.Stats.Readouts, 4);
  ASSERT_EQ(Parser.Stats.ErrorTimeFrac, 2);
}

// Testing valid and invalid BC ranges
TEST_F(VMM3ParserTest, ErrorBC) {
  makeHeader(VMMBCError);
  auto Res = Parser.parse(PacketData);
  ASSERT_EQ(Res, 2);
  ASSERT_EQ(Parser.Stats.Readouts, 4);
  ASSERT_EQ(Parser.Stats.ErrorBC, 2);
}

// Testing valid and invalid ADC ranges
TEST_F(VMM3ParserTest, ErrorADC) {
  makeHeader(VMMADCError);
  auto Res = Parser.parse(PacketData);
  ASSERT_EQ(Res, 2);
  ASSERT_EQ(Parser.Stats.Readouts, 4);
  ASSERT_EQ(Parser.Stats.ErrorADC, 2);
  ASSERT_EQ(Parser.Stats.OverThreshold, 1);
}

// Testing valid and invalid VMM ranges
TEST_F(VMM3ParserTest, ErrorVMM) {
  makeHeader(VMMVMMError);
  auto Res = Parser.parse(PacketData);
  ASSERT_EQ(Res, 2);
  ASSERT_EQ(Parser.Stats.Readouts, 4);
  ASSERT_EQ(Parser.Stats.ErrorVMM, 2);
}

// Testing valid and invalid VMM ranges
TEST_F(VMM3ParserTest, ErrorChannel) {
  makeHeader(VMMChannelError);
  auto Res = Parser.parse(PacketData);
  ASSERT_EQ(Res, 2);
  ASSERT_EQ(Parser.Stats.Readouts, 4);
  ASSERT_EQ(Parser.Stats.ErrorChannel, 2);
}


// valid data two readouts
TEST_F(VMM3ParserTest, GoodData1) {
  makeHeader(VMMData1);
  auto Res = Parser.parse(PacketData);
  ASSERT_EQ(Res, 2);
  ASSERT_EQ(Parser.Stats.Readouts, 2);
  ASSERT_EQ(Parser.Stats.DataReadouts, 2);
  ASSERT_EQ(Parser.Stats.CalibReadouts, 0);
}

// valid data two readouts
TEST_F(VMM3ParserTest, GoodCalib1) {
  makeHeader(VMMCalib1);
  auto Res = Parser.parse(PacketData);
  ASSERT_EQ(Res, 2);
  ASSERT_EQ(Parser.Stats.Readouts, 2);
  ASSERT_EQ(Parser.Stats.DataReadouts, 0);
  ASSERT_EQ(Parser.Stats.CalibReadouts, 2);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
