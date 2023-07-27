// Copyright (C) 2021 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief unit test for DREAM DataParser
///
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <dream/readout/DataParser.h>
#include <dream/test/DataParserTestData.h>

using namespace Dream;

class DataParserTest : public TestBase {
protected:
  // From Counters.h
  struct Counters Counters;
  DataParser Parser{Counters};
  void SetUp() override { Counters = {}; }
  void TearDown() override {}
};

TEST_F(DataParserTest, Constructor) {
  ASSERT_EQ(Parser.Stats.Readouts, 0);
  ASSERT_EQ(Parser.Stats.DataHeaders, 0);
  ASSERT_EQ(Parser.Stats.BufferErrors, 0);
  ASSERT_EQ(Parser.Stats.DataLenErrors, 0);
  ASSERT_EQ(Parser.Result.size(), 0);
}

TEST_F(DataParserTest, BadSize) {
  auto Res = Parser.parse((char *)&ErrBadRingGoodFEN[0], 3);
  ASSERT_EQ(Res, 0);
  ASSERT_EQ(Parser.Stats.Readouts, 0);
  ASSERT_EQ(Parser.Stats.DataHeaders, 0);
  ASSERT_EQ(Parser.Stats.BufferErrors, 1);
  ASSERT_EQ(Parser.Result.size(), 0);
}

TEST_F(DataParserTest, HeaderSizeError) {
  auto Res = Parser.parse((char *)&OkThreeDreamReadouts[0], 3);
  ASSERT_EQ(Res, 0);
  ASSERT_EQ(Parser.Stats.Readouts, 0);
  ASSERT_EQ(Parser.Stats.DataHeaders, 0);
  ASSERT_EQ(Parser.Stats.BufferErrors, 1);
  ASSERT_EQ(Parser.Result.size(), 0);
}

TEST_F(DataParserTest, BadRingGoodFEN) {
  auto Res = Parser.parse((char *)&ErrBadRingGoodFEN[0], 4);
  ASSERT_EQ(Res, 0);
  ASSERT_EQ(Parser.Stats.Readouts, 0);
  ASSERT_EQ(Parser.Stats.FiberErrors, 1);
  ASSERT_EQ(Parser.Result.size(), 0);
}

TEST_F(DataParserTest, GoodRingBadFEN) {
  auto Res = Parser.parse((char *)&ErrGoodRingBadFEN[0], 4);
  ASSERT_EQ(Res, 0);
  ASSERT_EQ(Parser.Stats.Readouts, 0);
  ASSERT_EQ(Parser.Stats.FENErrors, 1);
  ASSERT_EQ(Parser.Result.size(), 0);
}

TEST_F(DataParserTest, DataSizeMismatch) {
  auto Res = Parser.parse((char *)&OkThreeDreamReadouts[0], 10);
  ASSERT_EQ(Res, 0);
  ASSERT_EQ(Parser.Stats.Readouts, 0);
  ASSERT_EQ(Parser.Stats.BufferErrors, 1);
  ASSERT_EQ(Parser.Result.size(), 0);
}

TEST_F(DataParserTest, ParseThree) {
  auto Res = Parser.parse((char *)&OkThreeDreamReadouts[0],
                          OkThreeDreamReadouts.size());
  ASSERT_EQ(Res, 3);
  ASSERT_EQ(Parser.Stats.Readouts, 3);
  ASSERT_EQ(Parser.Stats.DataHeaders, 3);
  ASSERT_EQ(Parser.Stats.BufferErrors, 0);
  ASSERT_EQ(Parser.Stats.DataLenErrors, 0);
  ASSERT_EQ(Parser.Result.size(), 3);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
