// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief unit test for Bifrost DataParser
///
//===----------------------------------------------------------------------===//

#include <bifrost/readout/DataParser.h>
#include <bifrost/test/DataParserTestData.h>
#include <common/testutils/TestBase.h>

using namespace Bifrost;

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
  ASSERT_EQ(Parser.Stats.ErrorDataHeaders, 0);
  ASSERT_EQ(Parser.Stats.ErrorBytes, 0);
  ASSERT_EQ(Parser.Result.size(), 0);
}

TEST_F(DataParserTest, BadSize) {
  auto Res = Parser.parse((char *)&ErrBadRingGoodFEN[0], 3);
  ASSERT_EQ(Res, 0);
  ASSERT_EQ(Parser.Stats.Readouts, 0);
  ASSERT_EQ(Parser.Stats.DataHeaders, 0);
  ASSERT_EQ(Parser.Stats.ErrorDataHeaders, 1);
  ASSERT_EQ(Parser.Stats.ErrorBytes, 3);
  ASSERT_EQ(Parser.Result.size(), 0);
}

TEST_F(DataParserTest, HeaderSizeError) {
  auto Res = Parser.parse((char *)&ErrSizeMismatch[0], ErrSizeMismatch.size());
  ASSERT_EQ(Res, 0);
  ASSERT_EQ(Parser.Stats.Readouts, 0);
  ASSERT_EQ(Parser.Stats.DataHeaders, 1);
  ASSERT_EQ(Parser.Stats.ErrorDataHeaders, 1);
  ASSERT_EQ(Parser.Stats.ErrorBytes, ErrSizeMismatch.size());
  ASSERT_EQ(Parser.Result.size(), 0);
}

TEST_F(DataParserTest, BadRingGoodFEN) {
  auto Res = Parser.parse((char *)&ErrBadRingGoodFEN[0], 4);
  ASSERT_EQ(Res, 0);
  ASSERT_EQ(Parser.Stats.Readouts, 0);
  ASSERT_EQ(Parser.Stats.ErrorDataHeaders, 1);
  ASSERT_EQ(Parser.Stats.ErrorBytes, 4);
  ASSERT_EQ(Parser.Result.size(), 0);
}

TEST_F(DataParserTest, GoodRingBadFEN) {
  auto Res = Parser.parse((char *)&ErrGoodRingBadFEN[0], 4);
  ASSERT_EQ(Res, 0);
  ASSERT_EQ(Parser.Stats.Readouts, 0);
  ASSERT_EQ(Parser.Stats.ErrorDataHeaders, 1);
  ASSERT_EQ(Parser.Stats.ErrorBytes, 4);
  ASSERT_EQ(Parser.Result.size(), 0);
}

TEST_F(DataParserTest, DataSizeMismatch) {
  auto Res = Parser.parse((char *)&OkThreeBifrostReadouts[0], 10);
  ASSERT_EQ(Res, 0);
  ASSERT_EQ(Parser.Stats.Readouts, 0);
  ASSERT_EQ(Parser.Stats.ErrorDataHeaders, 1);
  ASSERT_EQ(Parser.Stats.ErrorBytes, 10);
  ASSERT_EQ(Parser.Result.size(), 0);
}

TEST_F(DataParserTest, ParseThree) {
  auto Res = Parser.parse((char *)&OkThreeBifrostReadouts[0],
                          OkThreeBifrostReadouts.size());
  ASSERT_EQ(Res, 3);
  ASSERT_EQ(Parser.Stats.Readouts, 3);
  ASSERT_EQ(Parser.Stats.DataHeaders, 3);
  ASSERT_EQ(Parser.Stats.ErrorDataHeaders, 0);
  ASSERT_EQ(Parser.Stats.ErrorBytes, 0);
  ASSERT_EQ(Parser.Result.size(), 3);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
