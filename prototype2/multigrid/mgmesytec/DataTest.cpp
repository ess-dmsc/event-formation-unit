/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

//#include <algorithm>
//#include <memory>
#include <multigrid/mgmesytec/Data.h>
#include <multigrid/mgmesytec/TestData.h>
#include <test/TestBase.h>

class MesytecDataTest : public TestBase {
protected:
  MesytecData mesytec;
  virtual void SetUp() { }
  virtual void TearDown() { }
};

/** Test cases below */

TEST_F(MesytecDataTest, ErrUnsupportedCommand) {
  auto res = mesytec.parse((char *)&err_unsupported_cmd[0], err_unsupported_cmd.size());

  ASSERT_EQ(res, -MesytecData::error::EUNSUPP);
}

TEST_F(MesytecDataTest, ErrPktShort) {
  auto res = mesytec.parse((char *)&err_pkt_too_short[0], err_pkt_too_short.size());

  ASSERT_EQ(res, -MesytecData::error::ESIZE);
}


TEST_F(MesytecDataTest, ParseRecordedWSData) {
  auto res = mesytec.parse((char *)&ws1[0], ws1.size());
  ASSERT_EQ(res, -MesytecData::error::OK);
  ASSERT_EQ(mesytec.readouts, 128);
}

TEST_F(MesytecDataTest, ParseRecordedWSDataII) {
  auto res = mesytec.parse((char *)&ws2[0], ws2.size());
  ASSERT_EQ(res, -MesytecData::error::OK);
  ASSERT_EQ(mesytec.readouts, 256);
}

TEST_F(MesytecDataTest, ParseRecordedWSDataIII) {
  auto res = mesytec.parse((char *)&ws3[0], ws3.size());
  ASSERT_EQ(res, -MesytecData::error::OK);
  ASSERT_EQ(mesytec.readouts, 256);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
