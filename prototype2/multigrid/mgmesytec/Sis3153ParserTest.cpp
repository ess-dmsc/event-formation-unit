/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/mgmesytec/Sis3153Parser.h>
#include <multigrid/mgmesytec/TestData.h>
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
  auto res = sis.parse(Buffer((char *)&err_unsupported_cmd[0], err_unsupported_cmd.size()));
  EXPECT_EQ(res, 0);
  EXPECT_EQ(sis.buffers.size(), 0);

  res = sis.parse(Buffer((char *)&err_unsupported_cmd_II[0], err_unsupported_cmd_II.size()));
  EXPECT_EQ(res, 0);
  EXPECT_EQ(sis.buffers.size(), 0);
}

TEST_F(Sis3153ParserTest, ErrNoSisReadoutHeader) {
  auto res = sis.parse(Buffer((char *)&err_no_sis_readout_header[0], err_no_sis_readout_header.size()));
  EXPECT_EQ(res, 0);
  EXPECT_EQ(sis.buffers.size(), 0);
}

TEST_F(Sis3153ParserTest, ErrNoSisReadoutTrailer) {
  auto res = sis.parse(Buffer((char *)&err_no_sis_readout_trailer[0], err_no_sis_readout_trailer.size()));
  EXPECT_EQ(res, 0);
  EXPECT_EQ(sis.buffers.size(), 1);
}

TEST_F(Sis3153ParserTest, ErrNoTimeStamp) {
  auto res = sis.parse(Buffer((char *)&err_no_timestamp[0], err_no_timestamp.size()));
  EXPECT_EQ(res, 1);
  EXPECT_EQ(sis.buffers.size(), 1);
}

TEST_F(Sis3153ParserTest, ErrNoEndDataCookie) {
  auto res = sis.parse(Buffer((char *)&err_no_end_data_cookie[0], err_no_end_data_cookie.size()));
  EXPECT_EQ(res, 0);
  EXPECT_EQ(sis.buffers.size(), 1);
}

TEST_F(Sis3153ParserTest, ErrPktShort) {
  auto res = sis.parse(Buffer((char *)&err_pkt_too_short[0], err_pkt_too_short.size()));
  EXPECT_EQ(res, 0);
  EXPECT_EQ(sis.buffers.size(), 0);
}

TEST_F(Sis3153ParserTest, ParseRecordedWSData) {
  auto res = sis.parse(Buffer((char *)&ws1[0], ws1.size()));
  EXPECT_EQ(res, 1);
  EXPECT_EQ(sis.buffers.size(), 1);
}

TEST_F(Sis3153ParserTest, ParseRecordedWSDataII) {
  auto res = sis.parse(Buffer((char *)&ws2[0], ws2.size()));
  EXPECT_EQ(res, 2);
  EXPECT_EQ(sis.buffers.size(), 2);
}

TEST_F(Sis3153ParserTest, ParseRecordedWSDataIII) {
  auto res = sis.parse(Buffer((char *)&ws3[0], ws3.size()));
  EXPECT_EQ(res, 4);
  EXPECT_EQ(sis.buffers.size(), 4);
}

TEST_F(Sis3153ParserTest, ParseRecordedWSDataMultipleTriggers) {
  auto res = sis.parse(Buffer((char *)&ws4[0], ws4.size()));
  EXPECT_EQ(res, 36);
  EXPECT_EQ(sis.buffers.size(), 36);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
