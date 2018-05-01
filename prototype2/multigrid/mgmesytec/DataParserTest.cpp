/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/nmx/Hists.h> // @fixme
#include <multigrid/mgmesytec/DataParser.h>
#include <multigrid/mgmesytec/TestData.h>
#include <test/TestBase.h>

static Producer producer {"noserver", "nostream"};

static const int MG24_Z_20 = 1;

class MesytecDataTest : public TestBase {
protected:
  NMXHists hists;
  #ifdef DUMPTOFILE
    MesytecData mesytec{0, "nofile", MG24_Z_20}; // Dont dumptofile select module with 20 depth in z
  #else
    MesytecData mesytec{MG24_Z_20};
  #endif
  ReadoutSerializer * serializer;
  FBSerializer * fbserializer;
  virtual void SetUp() {
    serializer = new ReadoutSerializer(10000, producer);
    fbserializer = new FBSerializer(1000000, producer);
  }
  virtual void TearDown() {
    delete serializer;
    delete fbserializer;
  }
};

/** Test cases below */
TEST_F(MesytecDataTest, ErrUnsupportedCommand) {
  auto res = mesytec.parse((char *)&err_unsupported_cmd[0], err_unsupported_cmd.size(), hists, *fbserializer, *serializer);
  ASSERT_EQ(res, MesytecData::error::EUNSUPP);

  res = mesytec.parse((char *)&err_unsupported_cmd_II[0], err_unsupported_cmd_II.size(), hists, *fbserializer, *serializer);
  ASSERT_EQ(res, MesytecData::error::EUNSUPP);
}

TEST_F(MesytecDataTest, ErrNoSisReadoutHeader) {
  auto res = mesytec.parse((char *)&err_no_sis_readout_header[0], err_no_sis_readout_header.size(), hists, *fbserializer, *serializer);
  ASSERT_EQ(res, MesytecData::error::EHEADER);
}

TEST_F(MesytecDataTest, ErrNoSisReadoutTrailer) {
  auto res = mesytec.parse((char *)&err_no_sis_readout_trailer[0], err_no_sis_readout_trailer.size(), hists, *fbserializer, *serializer);
  ASSERT_EQ(res, MesytecData::error::EHEADER);
}

TEST_F(MesytecDataTest, ErrNoTimeStamp) {
  auto res = mesytec.parse((char *)&err_no_timestamp[0], err_no_timestamp.size(), hists, *fbserializer, *serializer);
  ASSERT_EQ(res, MesytecData::error::OK);
  ASSERT_EQ(mesytec.readouts, 0);
}

TEST_F(MesytecDataTest, ErrNoEndDataCookie) {
  auto res = mesytec.parse((char *)&err_no_end_data_cookie[0], err_no_end_data_cookie.size(), hists, *fbserializer, *serializer);
  ASSERT_EQ(res, MesytecData::error::EHEADER);
}

TEST_F(MesytecDataTest, ErrPktShort) {
  auto res = mesytec.parse((char *)&err_pkt_too_short[0], err_pkt_too_short.size(), hists, *fbserializer, *serializer);

  ASSERT_EQ(res, MesytecData::error::ESIZE);
}


TEST_F(MesytecDataTest, ParseRecordedWSData) {
  auto res = mesytec.parse((char *)&ws1[0], ws1.size(), hists, *fbserializer, *serializer);
  ASSERT_EQ(res, MesytecData::error::OK);
  ASSERT_EQ(mesytec.readouts, 128);
  ASSERT_EQ(mesytec.discards, 0);
  ASSERT_EQ(mesytec.triggers, mesytec.geometry_errors + mesytec.events);
}

TEST_F(MesytecDataTest, ParseRecordedWSDataDiscardAll) {
  mesytec.setWireThreshold(60000, 65535);
  mesytec.setGridThreshold(60000, 65535);
  auto res = mesytec.parse((char *)&ws1[0], ws1.size(), hists, *fbserializer, *serializer);
  ASSERT_EQ(res, MesytecData::error::OK);
  ASSERT_EQ(mesytec.readouts, 128);
  ASSERT_EQ(mesytec.discards, 128);
  ASSERT_EQ(mesytec.triggers, mesytec.geometry_errors + mesytec.events);
}

TEST_F(MesytecDataTest, ParseRecordedWSDataII) {
  auto res = mesytec.parse((char *)&ws2[0], ws2.size(), hists, *fbserializer, *serializer);
  ASSERT_EQ(res, MesytecData::error::OK);
  ASSERT_EQ(mesytec.readouts, 256);
  ASSERT_EQ(mesytec.triggers, mesytec.geometry_errors + mesytec.events);
}

TEST_F(MesytecDataTest, ParseRecordedWSDataIII) {
  auto res = mesytec.parse((char *)&ws3[0], ws3.size(), hists, *fbserializer, *serializer);
  ASSERT_EQ(res, MesytecData::error::OK);
  ASSERT_EQ(mesytec.readouts, 256); // Readout provides more than 92 channels
  ASSERT_EQ(mesytec.triggers, mesytec.geometry_errors + mesytec.events);
}

TEST_F(MesytecDataTest, ParseRecordedWSDataMultipleTriggers) {
  auto res = mesytec.parse((char *)&ws4[0], ws4.size(), hists, *fbserializer, *serializer);
  ASSERT_EQ(res, MesytecData::error::OK);
  ASSERT_EQ(mesytec.triggers, 36);
  ASSERT_EQ(mesytec.readouts, 54);
  ASSERT_EQ(mesytec.discards, 0);
  ASSERT_EQ(mesytec.events, 23);
  ASSERT_EQ(mesytec.geometry_errors, 13);
  ASSERT_TRUE(mesytec.triggers == mesytec.geometry_errors + mesytec.events);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
