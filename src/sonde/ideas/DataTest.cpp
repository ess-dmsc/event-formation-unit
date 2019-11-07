/** Copyright (C) 2017 European Spallation Source ERIC */

#include <algorithm>
#include <memory>
#include <sonde/Geometry.h>
#include <sonde/ideas/Data.h>
#include <sonde/ideas/TestData.h>
#include <test/TestBase.h>

using namespace Sonde;

class IDEASDataTest : public TestBase {
protected:
  Geometry geometry;
  IDEASData *readout;
  void SetUp() override {
    readout = new IDEASData(&geometry, "deleteme_sonde_test_");
    memset(readout->data, 0, sizeof(readout->data));
  }
  void TearDown() override {
    delete readout;
  }
};

/** Test cases below */
TEST_F(IDEASDataTest, ErrNoData) {
  int res = readout->parse_buffer(0, 20);
  ASSERT_EQ(res, -IDEASData::EBUFFER);
}

TEST_F(IDEASDataTest, ErrHeaderFields) {
  int i = 0;
  for (auto v : err_hdr) {
    MESSAGE() << "err_hdr[" << i++ << "]\n";
    int res = readout->parse_buffer((char *)&v[0], v.size());
    ASSERT_EQ(res, -IDEASData::EHEADER);
  }
}

TEST_F(IDEASDataTest, ErrShortHeader) {
  int size = err_short_header.size();
  int res = readout->parse_buffer((char *)&err_short_header[0], size);
  ASSERT_EQ(res, -IDEASData::EBADSIZE);
}

TEST_F(IDEASDataTest, ErrUnknownDataFormat) {
  int size = err_unknown_data_format.size();
  int res = readout->parse_buffer((char *)&err_unknown_data_format[0], size);
  ASSERT_EQ(res, -IDEASData::EUNSUPP);
}

TEST_F(IDEASDataTest, ErrInvalidGeometry) {
  int size = err_invalid_geometry.size();
  int res = readout->parse_buffer((char *)&err_invalid_geometry[0], size);
  ASSERT_EQ(res, 0);
  ASSERT_EQ(readout->errors, 1);
}

TEST_F(IDEASDataTest, OkHeaderOnly) {
  int size = ok_header_only.size();
  int res = readout->parse_buffer((char *)&ok_header_only[0], size);
  ASSERT_EQ(res, 0);
  ASSERT_EQ(readout->events, 0);
}

TEST_F(IDEASDataTest, OkOneEvent) {
  int size = ok_events_1.size();
  int res = readout->parse_buffer((char *)&ok_events_1[0], size);
  ASSERT_EQ(res, 1);
  ASSERT_EQ(readout->events, 1);
}

TEST_F(IDEASDataTest, OkThreeEvents) {
  int size = ok_events_3.size();
  int res = readout->parse_buffer((char *)&ok_events_3[0], size);
  ASSERT_EQ(res, 3);
  ASSERT_EQ(readout->events, 3);

  for (int i = 0; i < res; i++) {
    ASSERT_NE(0, readout->data[i].Time);
    ASSERT_NE(0, readout->data[i].PixelId);
  }
}

TEST_F(IDEASDataTest, SEPHOkOneSample) {
  int size = type_0xd5_seph_ok_1.size();
  int res = readout->parse_buffer((char *)&type_0xd5_seph_ok_1[0], size);
  ASSERT_EQ(res, 1);
  ASSERT_EQ(readout->samples, 1);
}

TEST_F(IDEASDataTest, SEPHOkThreeSamples) {
  int size = type_0xd5_seph_ok_3.size();
  int res = readout->parse_buffer((char *)&type_0xd5_seph_ok_3[0], size);
  ASSERT_EQ(res, 3); // Currently returns one event per sample
  ASSERT_EQ(readout->samples, 3);
  ASSERT_EQ(readout->data[0].Adc, 0x1234);
  ASSERT_EQ(readout->data[1].Adc, 0x5678);
  ASSERT_EQ(readout->data[2].Adc, 0x9abc);
}

TEST_F(IDEASDataTest, SEPHErrHdrLenMismatch) {
  int size = type_0xd5_seph_err_hdr_len_mismatch.size();
  int res = readout->parse_buffer(
      (char *)&type_0xd5_seph_err_hdr_len_mismatch[0], size);
  ASSERT_EQ(res, -IDEASData::EHEADER);
}

TEST_F(IDEASDataTest, MEPHErrHdrLen) {
  int size = type_0xd4_meph_err_hdr_len.size();
  int res = readout->parse_buffer((char *)&type_0xd4_meph_err_hdr_len[0], size);
  ASSERT_EQ(res, -IDEASData::EBADSIZE);
}

TEST_F(IDEASDataTest, MEPHErrHdrLenMismatch) {
  int size = type_0xd4_meph_err_hdr_len_mismatch.size();
  int res = readout->parse_buffer(
      (char *)&type_0xd4_meph_err_hdr_len_mismatch[0], size);
  ASSERT_EQ(res, -IDEASData::EHEADER);
}

TEST_F(IDEASDataTest, MEPHOkOneSample) {
  int size = type_0xd4_meph_ok_1.size();
  int res = readout->parse_buffer((char *)&type_0xd4_meph_ok_1[0], size);
  ASSERT_EQ(res, 1);
  ASSERT_EQ(readout->samples, 1);
}

TEST_F(IDEASDataTest, MEPHOkN3M1) {
  int size = type_0xd4_meph_ok_n3m1.size();
  int res = readout->parse_buffer((char *)&type_0xd4_meph_ok_n3m1[0], size);
  ASSERT_EQ(res, 3); // Should always return 3 events
  // This also implicitly testes the data endianness
  ASSERT_EQ(readout->samples, 3);
  ASSERT_EQ(readout->data[0].Time, 0x00000001);
  ASSERT_EQ(readout->data[0].Adc, 0x1234);
  ASSERT_EQ(readout->data[1].Time, 0x00000005);
  ASSERT_EQ(readout->data[1].Adc, 0x5678);
  ASSERT_EQ(readout->data[2].Time, 0x00000009);
  ASSERT_EQ(readout->data[2].Adc, 0x9abc);
}

TEST_F(IDEASDataTest, SCRO_pkt) {
  int size = scro_pkt.size();
  int res = readout->parse_buffer((char *)&scro_pkt[0], size);
  ASSERT_EQ(res, 150); // There is really only one event but 150 samples
}

TEST_F(IDEASDataTest, ACRO_pkt) {
  int size = acro_pkt.size();
  int res = readout->parse_buffer((char *)&acro_pkt[0], size);
  ASSERT_EQ(res, 64); // There is really only one event but 1 sample per channel
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
