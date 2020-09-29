/** Copyright (C) 2016, 2017 European Spallation Source ERIC */


#include <gdgem/srs/ParserVMM3.h>
#include <gdgem/srs/ParserVMM3TestData.h>
#include <test/TestBase.h>
#include <vector>

using namespace Gem;

class ParserVMM3Test : public TestBase {

protected:
  NMXStats stats;
  std::unique_ptr<ParserVMM3> parser;
  SRSTime srsTime;
  void SetUp() override {
    srsTime.bc_clock_MHz(40);
    srsTime.tac_slope_ns(60);
    srsTime.trigger_resolution_ns(1);
    parser = std::make_unique<ParserVMM3>(1125, stats, srsTime);
  }

  void memSet() {
    parser->stats.ParserReadouts = 0;
    parser->stats.ParserData = 0;
    parser->stats.ParserMarkers = 0;
    parser->stats.ParserFrameMissingErrors = 0;
    parser->stats.ParserFrameSeqErrors = 0;
    parser->stats.ParserFramecounterOverflows = 0;
    parser->stats.ParserTimestampLostErrors = 0;
    parser->stats.ParserTimestampSeqErrors = 0;
    parser->stats.ParserTimestampOverflows = 0;
    parser->stats.ParserBadFrames = 0;
    parser->stats.ParserGoodFrames = 0;
    parser->stats.ParserErrorBytes = 0;
  }

  void assertfields(unsigned int hits, unsigned int markers, unsigned int errors) {
    EXPECT_EQ(parser->stats.ParserData, hits);
    EXPECT_EQ(parser->stats.ParserMarkers, markers);
    EXPECT_EQ(parser->stats.ParserErrorBytes, errors);
  }
};

/** Test cases below */
TEST_F(ParserVMM3Test, Constructor) {
  EXPECT_TRUE(parser->data != nullptr);
  assertfields(0, 0, 0);
  for (int i = 0; i < MaxVMMs*MaxFECs; i++) {
    EXPECT_EQ(parser->markers[i].fecTimeStamp, 0U);
  }
}

TEST_F(ParserVMM3Test, UndersizeData) {
  for (int dataLength = 0; dataLength <= 16; dataLength++) {
    memSet();
    int res = parser->receive((char *)data_3_ch0.data(), dataLength);
    EXPECT_EQ(res, 0);
    assertfields(0, 0, dataLength);
    for (int i = 0; i < MaxVMMs*MaxFECs; i++) {
      EXPECT_EQ(parser->markers[i].fecTimeStamp, 0U);
    }
  }
}

TEST_F(ParserVMM3Test, DataOnly) {
  memSet();
  int res = parser->receive((char *)data_3_ch0.data(), data_3_ch0.size());
  EXPECT_EQ(res, 3); // three readouts in the readout packet
  assertfields(3, 0, 0);
  for (int i = 0; i < MaxVMMs*MaxFECs; i++) {
    EXPECT_EQ(parser->markers[i].fecTimeStamp, 0U);
  }
}

TEST_F(ParserVMM3Test, MarkerOnly) {
  memSet();
  int res = parser->receive((char *)marker_3_vmm1_3.data(), marker_3_vmm1_3.size());
  EXPECT_EQ(res, 0);
  assertfields(0, 3, 0);
  for (int i = 0; i < 3; i++) {
    int vmmid = i + 1; // see test data
    EXPECT_EQ(parser->markers[vmmid].fecTimeStamp, i + 1);
  }
}

TEST_F(ParserVMM3Test, TimestampError) {
  memSet();
  int res = parser->receive((char *)timestamp_error.data(), timestamp_error.size());
  EXPECT_EQ(res, 0);
  EXPECT_EQ(0, parser->stats.ParserData);
  EXPECT_EQ(2, parser->stats.ParserMarkers);
  EXPECT_EQ(1, parser->stats.ParserTimestampSeqErrors);
}

TEST_F(ParserVMM3Test, TimestampOverflow) {
  memSet();
  int res = parser->receive((char *)timestamp_overflow.data(), timestamp_overflow.size());
  EXPECT_EQ(res, 0);
  EXPECT_EQ(0, parser->stats.ParserData);
  EXPECT_EQ(2, parser->stats.ParserMarkers);
  EXPECT_EQ(1, parser->stats.ParserTimestampOverflows);
}

TEST_F(ParserVMM3Test, TimestampLost) {
  memSet();
  int res = parser->receive((char *)timestamp_lost.data(), timestamp_lost.size());
  EXPECT_EQ(res, 2);
  EXPECT_EQ(2, parser->stats.ParserData);
  EXPECT_EQ(1, parser->stats.ParserMarkers);
  EXPECT_EQ(1, parser->stats.ParserTimestampLostErrors);
}

TEST_F(ParserVMM3Test, TimestampNotLost) {
  memSet();
  int res = parser->receive((char *)timestamp_not_lost.data(), timestamp_not_lost.size());
  EXPECT_EQ(res, 2);
  EXPECT_EQ(2, parser->stats.ParserData);
  EXPECT_EQ(2, parser->stats.ParserMarkers);
  EXPECT_EQ(0, parser->stats.ParserTimestampLostErrors);
}

TEST_F(ParserVMM3Test, FrameMissingError) {
  memSet();
  int res = parser->receive((char *)framecounter_error1.data(), framecounter_error1.size());
  EXPECT_EQ(res, 1);
  res = parser->receive((char *)framecounter_error2.data(), framecounter_error2.size());
  EXPECT_EQ(res, 1);
  EXPECT_EQ(2, parser->stats.ParserFrameMissingErrors);
}

TEST_F(ParserVMM3Test, FrameOrderError) {
  memSet();
  int res = parser->receive((char *)framecounter_error2.data(), framecounter_error2.size());
  EXPECT_EQ(res, 1);
  res = parser->receive((char *)framecounter_error1.data(), framecounter_error1.size());
  EXPECT_EQ(res, 1);
  EXPECT_EQ(1, parser->stats.ParserFrameSeqErrors);
}

TEST_F(ParserVMM3Test, FramecounterOverflow) {
  memSet();
  int res = parser->receive((char *)framecounter_overflow1.data(), framecounter_overflow1.size());
  EXPECT_EQ(res, 1);
  res = parser->receive((char *)framecounter_overflow2.data(), framecounter_overflow2.size());
  EXPECT_EQ(res, 1);
  EXPECT_EQ(1, parser->stats.ParserFramecounterOverflows);
}


TEST_F(ParserVMM3Test, MarkerAndData) {
  memSet();
  int res = parser->receive((char *)marker_3_data_3.data(), marker_3_data_3.size());
  EXPECT_EQ(res, 3); // three readouts in the readout packet
  assertfields(3, 3, 0);
  for (int i = 0; i < 3; i++) {
    int vmmid = i + 1; // see test data
    EXPECT_EQ(parser->markers[vmmid].fecTimeStamp, i + 1);
  }
}

TEST_F(ParserVMM3Test, MarkerAndDataMixed) {
  memSet();
  int res = parser->receive((char *)marker_data_mixed_3.data(), marker_data_mixed_3.size());
  EXPECT_EQ(res, 3); // three readouts in the readout packet
  assertfields(3, 3, 0);
  for (int i = 0; i < 3; i++) {
    int vmmid = i + 1; // see test data
    EXPECT_EQ(parser->markers[vmmid].fecTimeStamp, i + 1);
  }
}

TEST_F(ParserVMM3Test, NoData) {
  memSet();
  int res = parser->receive((char *)no_data.data(), no_data.size());
  EXPECT_EQ(res, 0);
  assertfields(0, 0, no_data.size());
}

TEST_F(ParserVMM3Test, InvalidDataId) {
  memSet();
  int res = parser->receive((char *)invalid_dataid.data(), invalid_dataid.size());
  EXPECT_EQ(res, 0);
  assertfields(0, 0, invalid_dataid.size());
}

TEST_F(ParserVMM3Test, InconsistentDataLength) {
  memSet();
  int res = parser->receive((char *)inconsistent_datalen.data(), inconsistent_datalen.size());
  EXPECT_EQ(res, 0);
  assertfields(0, 0, inconsistent_datalen.size());
}

TEST_F(ParserVMM3Test, InvalidFEC0) {
  memSet();
  int hits = parser->receive((char *)invalid_fec_id.data(), invalid_fec_id.size());
  EXPECT_EQ(hits, 0);
  EXPECT_EQ(parser->stats.ParserBadFrames, 1);
}

TEST_F(ParserVMM3Test, DataLengthOverflow) {
  NMXStats stats;
  SRSTime srsTime;
  srsTime.bc_clock_MHz(40);
  srsTime.tac_slope_ns(60);
  srsTime.trigger_resolution_ns(1);
  ParserVMM3 shortvmmbuffer(2,stats, srsTime);
  int res = shortvmmbuffer.receive((char *)data_3_ch0.data(),  data_3_ch0.size());
  EXPECT_EQ(res, 2);
  EXPECT_EQ(2, shortvmmbuffer.stats.ParserData);
  EXPECT_EQ(0, shortvmmbuffer.stats.ParserMarkers);
  EXPECT_EQ(6, shortvmmbuffer.stats.ParserErrorBytes);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
