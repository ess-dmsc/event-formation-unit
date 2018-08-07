/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Hists.h> // \todo
#include <multigrid/mgmesytec/Sis3153Parser.h>
#include <multigrid/mgmesytec/TestData.h>
#include <test/TestBase.h>

using namespace Multigrid;

class Sis3153ParserTest : public TestBase {
protected:
  Sis3153Parser mesytec;
  virtual void SetUp() {
  }
  virtual void TearDown() {
  }
};

/** Test cases below */
TEST_F(Sis3153ParserTest, ErrUnsupportedCommand) {
  auto res = mesytec.parse(Buffer((char *)&err_unsupported_cmd[0], err_unsupported_cmd.size()));
  ASSERT_EQ(res, Sis3153Parser::error::EUNSUPP);

  res = mesytec.parse(Buffer((char *)&err_unsupported_cmd_II[0], err_unsupported_cmd_II.size()));
  ASSERT_EQ(res, Sis3153Parser::error::EUNSUPP);
}

TEST_F(Sis3153ParserTest, ErrNoSisReadoutHeader) {
  auto res = mesytec.parse(Buffer((char *)&err_no_sis_readout_header[0], err_no_sis_readout_header.size()));
  ASSERT_EQ(res, Sis3153Parser::error::EHEADER);
}

TEST_F(Sis3153ParserTest, ErrNoSisReadoutTrailer) {
  auto res = mesytec.parse(Buffer((char *)&err_no_sis_readout_trailer[0], err_no_sis_readout_trailer.size()));
  ASSERT_EQ(res, Sis3153Parser::error::EHEADER);
}

TEST_F(Sis3153ParserTest, ErrNoTimeStamp) {
  auto res = mesytec.parse(Buffer((char *)&err_no_timestamp[0], err_no_timestamp.size()));
  ASSERT_EQ(res, Sis3153Parser::error::OK);
//  ASSERT_EQ(mesytec.stats.readouts, 128);
//  ASSERT_EQ(mesytec.stats.events, 0);
}

TEST_F(Sis3153ParserTest, ErrNoEndDataCookie) {
  auto res = mesytec.parse(Buffer((char *)&err_no_end_data_cookie[0], err_no_end_data_cookie.size()));
  ASSERT_EQ(res, Sis3153Parser::error::EHEADER);
}

TEST_F(Sis3153ParserTest, ErrPktShort) {
  auto res = mesytec.parse(Buffer((char *)&err_pkt_too_short[0], err_pkt_too_short.size()));

  ASSERT_EQ(res, Sis3153Parser::error::ESIZE);
}

TEST_F(Sis3153ParserTest, ParseRecordedWSData) {
  auto res = mesytec.parse(Buffer((char *)&ws1[0], ws1.size()));
  ASSERT_EQ(res, Sis3153Parser::error::OK);
//  ASSERT_EQ(mesytec.stats.readouts, 128);
//  ASSERT_EQ(mesytec.stats.discards, 0); //128
//  ASSERT_EQ(mesytec.stats.triggers, 1);
}

/*
TEST_F(Sis3153ParserTest, ParseRecordedWSDataDiscardAll) {
  mesytec.setWireThreshold(60000, 65535);
  mesytec.setGridThreshold(60000, 65535);
  auto res = mesytec.parse(Buffer((char *)&ws1[0], ws1.size()));
  ASSERT_EQ(res, Sis3153Parser::error::OK);
  ASSERT_EQ(mesytec.stats.readouts, 128);
  ASSERT_EQ(mesytec.stats.discards, 128);
  ASSERT_EQ(mesytec.stats.triggers, 1);
}
*/

TEST_F(Sis3153ParserTest, ParseRecordedWSDataII) {
  auto res = mesytec.parse(Buffer((char *)&ws2[0], ws2.size()));
  ASSERT_EQ(res, Sis3153Parser::error::OK);
//  ASSERT_EQ(mesytec.stats.readouts, 256);
//  ASSERT_EQ(mesytec.stats.triggers, 2);
}

TEST_F(Sis3153ParserTest, ParseRecordedWSDataIII) {
  auto res = mesytec.parse(Buffer((char *)&ws3[0], ws3.size()));
  ASSERT_EQ(res, Sis3153Parser::error::OK);
//  ASSERT_EQ(mesytec.stats.readouts, 256); // Readout provides more than 92 channels
//  ASSERT_EQ(mesytec.stats.triggers, 4); // data containg four sis readout blocks starting with 0x58
//  ASSERT_EQ(mesytec.stats.badtriggers, 0); //4
}

TEST_F(Sis3153ParserTest, ParseRecordedWSDataMultipleTriggers) {
  auto res = mesytec.parse(Buffer((char *)&ws4[0], ws4.size()));
  ASSERT_EQ(res, Sis3153Parser::error::OK);
//  ASSERT_EQ(mesytec.stats.triggers, 36);
//  ASSERT_EQ(mesytec.stats.readouts, 54);
//  ASSERT_EQ(mesytec.stats.discards, 0); //67? includes ext triggers?
//  ASSERT_EQ(mesytec.stats.events, 0);
//  ASSERT_EQ(mesytec.stats.geometry_errors, 0);
  //ASSERT_TRUE(mesytec.stats.triggers == mesytec.stats.badtriggers + mesytec.stats.events);
}

// \todo reinstroduce tests with actual reduction

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
