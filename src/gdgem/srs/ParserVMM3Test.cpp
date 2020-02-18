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
    parser->stats.parser_readouts = 0;
    parser->stats.parser_data = 0;
    parser->stats.parser_markers = 0;
    parser->stats.parser_frame_missing_errors = 0;
    parser->stats.parser_frame_seq_errors = 0;
    parser->stats.parser_framecounter_overflows = 0;
    parser->stats.parser_timestamp_lost_errors = 0;
    parser->stats.parser_timestamp_seq_errors = 0; 
    parser->stats.parser_timestamp_overflows = 0;
    parser->stats.parser_bad_frames = 0;
    parser->stats.parser_good_frames = 0;
    parser->stats.parser_error_bytes = 0;
  }

  void assertfields(unsigned int hits, unsigned int markers, unsigned int errors) {
    EXPECT_EQ(parser->stats.parser_data, hits);
    EXPECT_EQ(parser->stats.parser_markers, markers);
    EXPECT_EQ(parser->stats.parser_error_bytes, errors);
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
    int res = parser->receive((char *)&data_3_ch0[0], dataLength);
    EXPECT_EQ(res, 0);
    assertfields(0, 0, dataLength);
    for (int i = 0; i < MaxVMMs*MaxFECs; i++) {
      EXPECT_EQ(parser->markers[i].fecTimeStamp, 0U);
    }
  }
}

TEST_F(ParserVMM3Test, DataOnly) {
  memSet();
  int res = parser->receive((char *)&data_3_ch0[0], data_3_ch0.size());
  EXPECT_EQ(res, 3); // three readouts in the readout packet
  assertfields(3, 0, 0);
  for (int i = 0; i < MaxVMMs*MaxFECs; i++) {
    EXPECT_EQ(parser->markers[i].fecTimeStamp, 0U);
  }
}

TEST_F(ParserVMM3Test, MarkerOnly) {
  memSet();
  int res = parser->receive((char *)&marker_3_vmm1_3[0], marker_3_vmm1_3.size());
  EXPECT_EQ(res, 0);
  assertfields(0, 3, 0);
  for (int i = 0; i < 3; i++) {
    int vmmid = i + 1; // see test data
    EXPECT_EQ(parser->markers[vmmid].fecTimeStamp, i + 1);
  }
}

TEST_F(ParserVMM3Test, TimestampError) {
  memSet();
  int res = parser->receive((char *)&timestamp_error[0], timestamp_error.size());
  EXPECT_EQ(res, 0);
  EXPECT_EQ(0, parser->stats.parser_data);
  EXPECT_EQ(2, parser->stats.parser_markers);
  EXPECT_EQ(1, parser->stats.parser_timestamp_seq_errors);
}

TEST_F(ParserVMM3Test, TimestampOverflow) {
  memSet();
  int res = parser->receive((char *)&timestamp_overflow[0], timestamp_overflow.size());
  EXPECT_EQ(res, 0);
  EXPECT_EQ(0, parser->stats.parser_data);
  EXPECT_EQ(2, parser->stats.parser_markers);
  EXPECT_EQ(1, parser->stats.parser_timestamp_overflows);
}

TEST_F(ParserVMM3Test, TimestampLost) {
  memSet();
  int res = parser->receive((char *)&timestamp_lost[0], timestamp_lost.size());
  EXPECT_EQ(res, 2);
  EXPECT_EQ(2, parser->stats.parser_data);
  EXPECT_EQ(1, parser->stats.parser_markers);
  EXPECT_EQ(1, parser->stats.parser_timestamp_lost_errors);
}

TEST_F(ParserVMM3Test, TimestampNotLost) {
  memSet();
  int res = parser->receive((char *)&timestamp_not_lost[0], timestamp_not_lost.size());
  EXPECT_EQ(res, 2);
  EXPECT_EQ(2, parser->stats.parser_data);
  EXPECT_EQ(2, parser->stats.parser_markers);
  EXPECT_EQ(0, parser->stats.parser_timestamp_lost_errors);
}

TEST_F(ParserVMM3Test, FrameMissingError) {
  memSet();
  int res = parser->receive((char *)&framecounter_error1[0], framecounter_error1.size());
  EXPECT_EQ(res, 1);
  res = parser->receive((char *)&framecounter_error2[0], framecounter_error2.size());
  EXPECT_EQ(res, 1);
  EXPECT_EQ(2, parser->stats.parser_frame_missing_errors);
}

TEST_F(ParserVMM3Test, FrameOrderError) {
  memSet();
  int res = parser->receive((char *)&framecounter_error2[0], framecounter_error2.size());
  EXPECT_EQ(res, 1);
  res = parser->receive((char *)&framecounter_error1[0], framecounter_error1.size());
  EXPECT_EQ(res, 1);
  EXPECT_EQ(1, parser->stats.parser_frame_seq_errors);
}

TEST_F(ParserVMM3Test, FramecounterOverflow) {
  memSet();
  int res = parser->receive((char *)&framecounter_overflow1[0], framecounter_overflow1.size());
  EXPECT_EQ(res, 1);
  res = parser->receive((char *)&framecounter_overflow2[0], framecounter_overflow2.size());
  EXPECT_EQ(res, 1);
  EXPECT_EQ(1, parser->stats.parser_framecounter_overflows);
}


TEST_F(ParserVMM3Test, MarkerAndData) {
  memSet();
  int res = parser->receive((char *)&marker_3_data_3[0], marker_3_data_3.size());
  EXPECT_EQ(res, 3); // three readouts in the readout packet
  assertfields(3, 3, 0);
  for (int i = 0; i < 3; i++) {
    int vmmid = i + 1; // see test data
    EXPECT_EQ(parser->markers[vmmid].fecTimeStamp, i + 1);
  }
}

TEST_F(ParserVMM3Test, MarkerAndDataMixed) {
  memSet();
  int res = parser->receive((char *)&marker_data_mixed_3[0], marker_data_mixed_3.size());
  EXPECT_EQ(res, 3); // three readouts in the readout packet
  assertfields(3, 3, 0);
  for (int i = 0; i < 3; i++) {
    int vmmid = i + 1; // see test data
    EXPECT_EQ(parser->markers[vmmid].fecTimeStamp, i + 1);
  }
}

TEST_F(ParserVMM3Test, NoData) {
  memSet();
  int res = parser->receive((char *)&no_data[0], no_data.size());
  EXPECT_EQ(res, 0);
  assertfields(0, 0, no_data.size());
}

TEST_F(ParserVMM3Test, InvalidDataId) {
  memSet();
  int res = parser->receive((char *)&invalid_dataid[0], invalid_dataid.size());
  EXPECT_EQ(res, 0);
  assertfields(0, 0, invalid_dataid.size());
}

TEST_F(ParserVMM3Test, InconsistentDataLength) {
  memSet();
  int res = parser->receive((char *)&inconsistent_datalen[0], inconsistent_datalen.size());
  EXPECT_EQ(res, 0);
  assertfields(0, 0, inconsistent_datalen.size());
}

TEST_F(ParserVMM3Test, DataLengthOverflow) {
  NMXStats stats;
  SRSTime srsTime;
  srsTime.bc_clock_MHz(40);
  srsTime.tac_slope_ns(60);
  srsTime.trigger_resolution_ns(1);
  ParserVMM3 shortvmmbuffer(2,stats, srsTime);
  int res = shortvmmbuffer.receive((char *)& data_3_ch0[0],  data_3_ch0.size());
  EXPECT_EQ(res, 2);
  EXPECT_EQ(2, shortvmmbuffer.stats.parser_data);
  EXPECT_EQ(0, shortvmmbuffer.stats.parser_markers);
  EXPECT_EQ(6, shortvmmbuffer.stats.parser_error_bytes);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
