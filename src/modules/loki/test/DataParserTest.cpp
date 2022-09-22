/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/testutils/TestBase.h>
#include <loki/readout/DataParser.h>
#include <loki/test/DataParserTestData.h>

using namespace Loki;

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
  auto Res = Parser.parse((char *)&OkLokiReadout[0], 10);
  ASSERT_EQ(Res, 0);
  ASSERT_EQ(Parser.Stats.Readouts, 0);
  ASSERT_EQ(Parser.Stats.ErrorDataHeaders, 1);
  ASSERT_EQ(Parser.Stats.ErrorBytes, 10);
  ASSERT_EQ(Parser.Result.size(), 0);
}

TEST_F(DataParserTest, ParseOne) {
  auto Res = Parser.parse((char *)&OkLokiReadout[0], OkLokiReadout.size());
  ASSERT_EQ(Res, 1);
  ASSERT_EQ(Parser.Stats.Readouts, 1);
  ASSERT_EQ(Parser.Stats.DataHeaders, 1);
  ASSERT_EQ(Parser.Stats.ErrorDataHeaders, 0);
  ASSERT_EQ(Parser.Stats.ErrorBytes, 0);
  ASSERT_EQ(Parser.Result.size(), 1);
}

TEST_F(DataParserTest, MultipleDataSection) {
  auto Res = Parser.parse((char *)&Ok2xLokiReadout[0], Ok2xLokiReadout.size());
  ASSERT_EQ(Res, 2);
  ASSERT_EQ(Parser.Stats.Readouts, 2);
  ASSERT_EQ(Parser.Stats.DataHeaders, 2);
  ASSERT_EQ(Parser.Stats.ErrorDataHeaders, 0);
  ASSERT_EQ(Parser.Stats.ErrorBytes, 0);
  ASSERT_EQ(Parser.Result.size(), 2);
}

TEST_F(DataParserTest, MultipleDataPackets) {
  auto Res = Parser.parse((char *)&Ok2xLokiReadout[0], Ok2xLokiReadout.size());
  ASSERT_EQ(Res, 2);
  ASSERT_EQ(Parser.Stats.Readouts, 2);
  ASSERT_EQ(Parser.Stats.DataHeaders, 2);
  ASSERT_EQ(Parser.Stats.ErrorDataHeaders, 0);
  ASSERT_EQ(Parser.Stats.ErrorBytes, 0);
  ASSERT_EQ(Parser.Result.size(), 2);

  Res = Parser.parse((char *)&Ok2xLokiReadout[0], Ok2xLokiReadout.size());
  ASSERT_EQ(Res, 2);
  ASSERT_EQ(Parser.Stats.Readouts, 4);
  ASSERT_EQ(Parser.Stats.DataHeaders, 4);
  ASSERT_EQ(Parser.Stats.ErrorDataHeaders, 0);
  ASSERT_EQ(Parser.Stats.ErrorBytes, 0);
  ASSERT_EQ(Parser.Result.size(), 2);
}

/// \todo confirm this response when passed too many readouts
TEST_F(DataParserTest, BadThreeReadouts) {
  auto Res = Parser.parse((char *)&ErrThreeLokiReadouts[0],
                          ErrThreeLokiReadouts.size());
  ASSERT_EQ(Res, 0);
  ASSERT_EQ(Parser.Stats.Readouts, 0);
  ASSERT_EQ(Parser.Stats.DataHeaders, 1);
  ASSERT_EQ(Parser.Stats.ErrorDataHeaders, 1);
  ASSERT_EQ(Parser.Stats.ErrorBytes, 64);
  ASSERT_EQ(Parser.Result.size(), 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
