// Copyright (C) 2016 - 2022 European Spallation Source ERIC

#include <caen/readout/DataParser.h>
#include <caen/test/DataParserTestData.h>
#include <common/testutils/TestBase.h>

using namespace Caen;

class DataParserTest : public TestBase {
protected:
  DataParser Parser;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(DataParserTest, Constructor) {
  ASSERT_EQ(Parser.Stats.Readouts, 0);
  ASSERT_EQ(Parser.Stats.DataHeaders, 0);
  ASSERT_EQ(Parser.Stats.DataHeaderSizeErrors, 0);
  ASSERT_EQ(Parser.Stats.RingFenErrors, 0);
  ASSERT_EQ(Parser.Stats.DataLenMismatch, 0);
  ASSERT_EQ(Parser.Stats.DataLenInvalid, 0);
  ASSERT_EQ(Parser.Result.size(), 0);
}

TEST_F(DataParserTest, BadSize) {
  auto Res = Parser.parse((char *)&ErrBadRingGoodFEN[0], 3);
  ASSERT_EQ(Res, 0);
  ASSERT_EQ(Parser.Stats.Readouts, 0);
  ASSERT_EQ(Parser.Stats.DataHeaders, 0);
  ASSERT_EQ(Parser.Stats.DataHeaderSizeErrors, 1);
  ASSERT_EQ(Parser.Result.size(), 0);
}

TEST_F(DataParserTest, HeaderSizeError) {
  auto Res = Parser.parse((char *)&ErrSizeMismatch[0], ErrSizeMismatch.size());
  ASSERT_EQ(Res, 0);
  ASSERT_EQ(Parser.Stats.Readouts, 0);
  ASSERT_EQ(Parser.Stats.DataHeaders, 1);
  ASSERT_EQ(Parser.Stats.DataLenInvalid, 1);
  ASSERT_EQ(Parser.Result.size(), 0);
}


TEST_F(DataParserTest, BadRingGoodFEN) {
  auto Res = Parser.parse((char *)&ErrBadRingGoodFEN[0], ErrBadRingGoodFEN.size());
  ASSERT_EQ(Res, 0);
  ASSERT_EQ(Parser.Stats.Readouts, 0);
  ASSERT_EQ(Parser.Stats.RingFenErrors, 1);
  ASSERT_EQ(Parser.Result.size(), 0);
}


TEST_F(DataParserTest, GoodRingBadFEN) {
  auto Res = Parser.parse((char *)&ErrGoodRingBadFEN[0], ErrGoodRingBadFEN.size());
  ASSERT_EQ(Res, 0);
  ASSERT_EQ(Parser.Stats.Readouts, 0);
  ASSERT_EQ(Parser.Stats.RingFenErrors, 1);
  ASSERT_EQ(Parser.Result.size(), 0);
}


TEST_F(DataParserTest, DataSizeMismatch) {
  auto Res = Parser.parse((char *)&OkCaenReadout[0], 10);
  ASSERT_EQ(Res, 0);
  ASSERT_EQ(Parser.Stats.Readouts, 0);
  ASSERT_EQ(Parser.Stats.DataLenMismatch, 1);
  ASSERT_EQ(Parser.Result.size(), 0);
}


TEST_F(DataParserTest, ParseOne) {
  auto Res = Parser.parse((char *)&OkCaenReadout[0], OkCaenReadout.size());
  ASSERT_EQ(Res, 1);
  ASSERT_EQ(Parser.Stats.Readouts, 1);
  ASSERT_EQ(Parser.Stats.DataHeaders, 1);
  ASSERT_EQ(Parser.Stats.DataHeaderSizeErrors, 0);
  ASSERT_EQ(Parser.Stats.RingFenErrors, 0);
  ASSERT_EQ(Parser.Stats.DataLenMismatch, 0);
  ASSERT_EQ(Parser.Stats.DataLenInvalid, 0);
  ASSERT_EQ(Parser.Result.size(), 1);
}


TEST_F(DataParserTest, MultipleDataSection) {
  auto Res = Parser.parse((char *)&Ok2xCaenReadout[0], Ok2xCaenReadout.size());
  ASSERT_EQ(Res, 2);
  ASSERT_EQ(Parser.Stats.Readouts, 2);
  ASSERT_EQ(Parser.Stats.DataHeaders, 2);
  ASSERT_EQ(Parser.Stats.DataHeaderSizeErrors, 0);
  ASSERT_EQ(Parser.Stats.RingFenErrors, 0);
  ASSERT_EQ(Parser.Stats.DataLenMismatch, 0);
  ASSERT_EQ(Parser.Stats.DataLenInvalid, 0);
  ASSERT_EQ(Parser.Result.size(), 2);
}

TEST_F(DataParserTest, MultipleDataPackets) {
  auto Res = Parser.parse((char *)&Ok2xCaenReadout[0], Ok2xCaenReadout.size());
  ASSERT_EQ(Res, 2);
  ASSERT_EQ(Parser.Stats.Readouts, 2);
  ASSERT_EQ(Parser.Stats.DataHeaders, 2);
  ASSERT_EQ(Parser.Stats.DataHeaderSizeErrors, 0);
  ASSERT_EQ(Parser.Stats.RingFenErrors, 0);
  ASSERT_EQ(Parser.Stats.DataLenMismatch, 0);
  ASSERT_EQ(Parser.Stats.DataLenInvalid, 0);
  ASSERT_EQ(Parser.Result.size(), 2);

  Res = Parser.parse((char *)&Ok2xCaenReadout[0], Ok2xCaenReadout.size());
  ASSERT_EQ(Res, 2);
  ASSERT_EQ(Parser.Stats.Readouts, 4);
  ASSERT_EQ(Parser.Stats.DataHeaders, 4);
  ASSERT_EQ(Parser.Stats.DataHeaderSizeErrors, 0);
  ASSERT_EQ(Parser.Stats.RingFenErrors, 0);
  ASSERT_EQ(Parser.Stats.DataLenMismatch, 0);
  ASSERT_EQ(Parser.Stats.DataLenInvalid, 0);
  ASSERT_EQ(Parser.Result.size(), 2);
}


/// \todo confirm this response when passed too many readouts
TEST_F(DataParserTest, BadThreeReadouts) {
  auto Res = Parser.parse((char *)&ErrThreeCaenReadouts[0],
                          ErrThreeCaenReadouts.size());
  ASSERT_EQ(Res, 0);
  ASSERT_EQ(Parser.Stats.Readouts, 0);
  ASSERT_EQ(Parser.Stats.DataHeaders, 1);
  ASSERT_EQ(Parser.Stats.DataLenInvalid, 1);
  ASSERT_EQ(Parser.Result.size(), 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
