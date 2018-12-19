/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/parsers/Sis3153Parser.h>
#include <multigrid/parsers/TestData.h>
#include <test/TestBase.h>

using namespace Multigrid;

class Sis3153ParserTest : public TestBase {
protected:
  Sis3153Parser sis;
  virtual void SetUp() {
  }
  virtual void TearDown() {
  }
};

/** Test cases below */
TEST_F(Sis3153ParserTest, ErrUnsupportedCommand) {
  auto res = sis.parse(err_unsupported_cmd);
  EXPECT_EQ(res, 22);
  EXPECT_EQ(sis.buffers.size(), 0);

  res = sis.parse(Buffer<uint8_t>(err_unsupported_cmd_II));
  EXPECT_EQ(res, 0);
  EXPECT_EQ(sis.buffers.size(), 0);
}

TEST_F(Sis3153ParserTest, ErrNoSisReadoutHeader) {
  auto res = sis.parse(err_no_sis_readout_header);
  EXPECT_EQ(res, 0);
  EXPECT_EQ(sis.buffers.size(), 0);
}

TEST_F(Sis3153ParserTest, ErrNoSisReadoutTrailer) {
  auto res = sis.parse(err_no_sis_readout_trailer);
  EXPECT_EQ(res, 4);
  EXPECT_EQ(sis.buffers.size(), 1);
}

TEST_F(Sis3153ParserTest, ErrNoTimeStamp) {
  auto res = sis.parse(err_no_timestamp);
  EXPECT_EQ(res, 0);
  EXPECT_EQ(sis.buffers.size(), 1);
}

TEST_F(Sis3153ParserTest, ErrNoEndDataCookie) {
  auto res = sis.parse(err_no_end_data_cookie);
  EXPECT_EQ(res, 8);
  EXPECT_EQ(sis.buffers.size(), 1);
}

TEST_F(Sis3153ParserTest, ErrPktShort) {
  auto res = sis.parse(err_pkt_too_short);
  EXPECT_EQ(res, 10);
  EXPECT_EQ(sis.buffers.size(), 0);
}

TEST_F(Sis3153ParserTest, ParseRecordedWSData) {
  auto res = sis.parse(ws1);
  EXPECT_EQ(res, 0);
  EXPECT_EQ(sis.buffers.size(), 1);
}

TEST_F(Sis3153ParserTest, ParseRecordedWSDataII) {
  auto res = sis.parse(ws2);
  EXPECT_EQ(res, 0);
  EXPECT_EQ(sis.buffers.size(), 2);
}

TEST_F(Sis3153ParserTest, ParseRecordedWSDataIII) {
  auto res = sis.parse(ws3);
  EXPECT_EQ(res, 0);
  EXPECT_EQ(sis.buffers.size(), 4);
}

TEST_F(Sis3153ParserTest, ParseRecordedWSDataMultipleTriggers) {
  auto res = sis.parse(ws4);
  EXPECT_EQ(res, 0);
  EXPECT_EQ(sis.buffers.size(), 36);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
