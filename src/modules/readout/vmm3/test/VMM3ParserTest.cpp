// Copyright (C) 2017-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for ReadoutParser
///
//===----------------------------------------------------------------------===//

#include <readout/vmm3/VMM3Parser.h>
#include <readout/vmm3/test/VMM3ParserTestData.h>
#include <test/TestBase.h>


class VMM3ParserTest : public TestBase {
protected:
  VMM3Parser Parser;
  void SetUp() override {}
  void TearDown() override {}
};


TEST_F(VMM3ParserTest, Constructor) {
  ASSERT_EQ(Parser.Result.size(), 0);
}

// nullptr as buffer
TEST_F(VMM3ParserTest, ErrorBufferPtr) {
  auto Res = Parser.parse(0, 100);
  ASSERT_EQ(Res, 0);
  ASSERT_EQ(Parser.Stats.ErrorSize, 1);
  ASSERT_EQ(Parser.Stats.Readouts, 0);
}

// invalid data size
TEST_F(VMM3ParserTest, ErrorAPISize) {
  auto Res = Parser.parse((char *)&VMMData1[0], 19);
  ASSERT_EQ(Res, 0);
  ASSERT_EQ(Parser.Stats.ErrorSize, 1);
  ASSERT_EQ(Parser.Stats.Readouts, 0);
}

// Invalid RingID
TEST_F(VMM3ParserTest, ErrorRing) {
  auto Res = Parser.parse((char *)&VMMRingError[0], VMMRingError.size());
  ASSERT_EQ(Res, 1);
  ASSERT_EQ(Parser.Stats.ErrorSize, 0);
  ASSERT_EQ(Parser.Stats.Readouts, 2);
  ASSERT_EQ(Parser.Stats.ErrorRing, 1);
  ASSERT_EQ(Parser.Stats.ErrorFEN, 0);
  ASSERT_EQ(Parser.Stats.ErrorDataLength, 0);
}

// Invalid FENId
TEST_F(VMM3ParserTest, ErrorFEN) {
  auto Res = Parser.parse((char *)&VMMFENError[0], VMMFENError.size());
  ASSERT_EQ(Res, 1);
  ASSERT_EQ(Parser.Stats.ErrorSize, 0);
  ASSERT_EQ(Parser.Stats.Readouts, 2);
  ASSERT_EQ(Parser.Stats.ErrorRing, 0);
  ASSERT_EQ(Parser.Stats.ErrorFEN, 1);
  ASSERT_EQ(Parser.Stats.ErrorDataLength, 0);
}

// Invalid data size
TEST_F(VMM3ParserTest, ErrorDataLength) {
  auto Res = Parser.parse((char *)&VMMDataLengthError[0], VMMDataLengthError.size());
  ASSERT_EQ(Res, 1);
  ASSERT_EQ(Parser.Stats.ErrorSize, 0);
  ASSERT_EQ(Parser.Stats.Readouts, 2);
  ASSERT_EQ(Parser.Stats.ErrorRing, 0);
  ASSERT_EQ(Parser.Stats.ErrorFEN, 0);
  ASSERT_EQ(Parser.Stats.ErrorDataLength, 1);
}



// valid data two readouts
TEST_F(VMM3ParserTest, GoodData1) {
  auto Res = Parser.parse((char *)&VMMData1[0], VMMData1.size());
  ASSERT_EQ(Res, 2);
  ASSERT_EQ(Parser.Stats.ErrorSize, 0);
  ASSERT_EQ(Parser.Stats.Readouts, 2);
  ASSERT_EQ(Parser.Stats.DataReadout, 2);
  ASSERT_EQ(Parser.Stats.CalibReadout, 0);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
