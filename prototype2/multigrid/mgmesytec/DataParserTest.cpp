/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Hists.h> // \todo
#include <multigrid/mgmesytec/DataParser.h>
#include <multigrid/mgmesytec/TestData.h>
#include <test/TestBase.h>

#include <multigrid/mgmesytec/MgSeqGeometry.h>

static Producer producer {"noserver", "nostream"};

static const int MG24_Z_20 = 1;

class MesytecDataTest : public TestBase {
protected:
  std::shared_ptr<MgSeqGeometry> geom;
  MesytecData *mesytec;
  FBSerializer * fbserializer;
  virtual void SetUp() {
    geom = std::make_shared<MgSeqGeometry>();
    geom->select_module(MG24_Z_20);
    geom->swap_on(true);
    MgEFU mgefu(geom, nullptr);
    mesytec = new MesytecData(mgefu, nullptr);
    fbserializer = new FBSerializer(1000000, producer);
  }
  virtual void TearDown() {
    delete mesytec;
    delete fbserializer;
  }
};

/** Test cases below */
TEST_F(MesytecDataTest, ErrUnsupportedCommand) {
  auto res = mesytec->parse((char *)&err_unsupported_cmd[0], err_unsupported_cmd.size(), *fbserializer);
  ASSERT_EQ(res, MesytecData::error::EUNSUPP);

  res = mesytec->parse((char *)&err_unsupported_cmd_II[0], err_unsupported_cmd_II.size(), *fbserializer);
  ASSERT_EQ(res, MesytecData::error::EUNSUPP);
}

TEST_F(MesytecDataTest, ErrNoSisReadoutHeader) {
  auto res = mesytec->parse((char *)&err_no_sis_readout_header[0], err_no_sis_readout_header.size(), *fbserializer);
  ASSERT_EQ(res, MesytecData::error::EHEADER);
}

TEST_F(MesytecDataTest, ErrNoSisReadoutTrailer) {
  auto res = mesytec->parse((char *)&err_no_sis_readout_trailer[0], err_no_sis_readout_trailer.size(), *fbserializer);
  ASSERT_EQ(res, MesytecData::error::EHEADER);
}

TEST_F(MesytecDataTest, ErrNoTimeStamp) {
  auto res = mesytec->parse((char *)&err_no_timestamp[0], err_no_timestamp.size(), *fbserializer);
  ASSERT_EQ(res, MesytecData::error::OK);
  ASSERT_EQ(mesytec->stats.readouts, 128);
  ASSERT_EQ(mesytec->stats.events, 0);
}

TEST_F(MesytecDataTest, ErrNoEndDataCookie) {
  auto res = mesytec->parse((char *)&err_no_end_data_cookie[0], err_no_end_data_cookie.size(), *fbserializer);
  ASSERT_EQ(res, MesytecData::error::EHEADER);
}

TEST_F(MesytecDataTest, ErrPktShort) {
  auto res = mesytec->parse((char *)&err_pkt_too_short[0], err_pkt_too_short.size(), *fbserializer);

  ASSERT_EQ(res, MesytecData::error::ESIZE);
}

TEST_F(MesytecDataTest, ParseRecordedWSData) {
  auto res = mesytec->parse((char *)&ws1[0], ws1.size(), *fbserializer);
  ASSERT_EQ(res, MesytecData::error::OK);
  ASSERT_EQ(mesytec->stats.readouts, 128);
  ASSERT_EQ(mesytec->stats.discards, 0);
  ASSERT_EQ(mesytec->stats.triggers, 1);
}

/*
TEST_F(MesytecDataTest, ParseRecordedWSDataDiscardAll) {
  mesytec->setWireThreshold(60000, 65535);
  mesytec->setGridThreshold(60000, 65535);
  auto res = mesytec->parse((char *)&ws1[0], ws1.size(), *fbserializer);
  ASSERT_EQ(res, MesytecData::error::OK);
  ASSERT_EQ(mesytec->stats.readouts, 128);
  ASSERT_EQ(mesytec->stats.discards, 128);
  ASSERT_EQ(mesytec->stats.triggers, 1);
}
 */

TEST_F(MesytecDataTest, ParseRecordedWSDataII) {
  auto res = mesytec->parse((char *)&ws2[0], ws2.size(), *fbserializer);
  ASSERT_EQ(res, MesytecData::error::OK);
  ASSERT_EQ(mesytec->stats.readouts, 256);
  ASSERT_EQ(mesytec->stats.triggers, mesytec->stats.events);
}

TEST_F(MesytecDataTest, ParseRecordedWSDataIII) {
  auto res = mesytec->parse((char *)&ws3[0], ws3.size(), *fbserializer);
  ASSERT_EQ(res, MesytecData::error::OK);
  ASSERT_EQ(mesytec->stats.readouts, 256); // Readout provides more than 92 channels
  ASSERT_EQ(mesytec->stats.triggers, 4); // data containg four sis readout blocks starting with 0x58
  ASSERT_EQ(mesytec->stats.badtriggers, 2);
}

TEST_F(MesytecDataTest, ParseRecordedWSDataMultipleTriggers) {
  auto res = mesytec->parse((char *)&ws4[0], ws4.size(), *fbserializer);
  ASSERT_EQ(res, MesytecData::error::OK);
  ASSERT_EQ(mesytec->stats.triggers, 36);
  ASSERT_EQ(mesytec->stats.readouts, 54);
  ASSERT_EQ(mesytec->stats.discards, 0);
  ASSERT_EQ(mesytec->stats.events, 23);
  ASSERT_EQ(mesytec->stats.geometry_errors, 0);
  ASSERT_TRUE(mesytec->stats.triggers == mesytec->stats.badtriggers + mesytec->stats.events);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
