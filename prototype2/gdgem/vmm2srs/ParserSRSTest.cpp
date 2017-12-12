/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/vmm2srs/ParserSRS.h>
#include <test/TestBase.h>
#include <vector>

// clang-format off
std::vector<uint8_t> err_usize1   { 0x40 };

std::vector<uint8_t> err_usize3   { 0x40, 0x41, 0x42 };

std::vector<uint8_t> err_usize4   { 0xfc, 0xfc, 0xfc, 0xfc };

std::vector<uint8_t> err_usize7   { 0x40, 0x41, 0x42, 0x11, 0x01, 0x02, 0x03 };

std::vector<uint8_t> err_usize12  { 0x00, 0x00, 0x00, 0x01, 0x56, 0x4d, 0x32, 0x00, 0x01, 0x00, 0x00, 0x00 };

std::vector<uint8_t> no_data      { 0x00, 0x00, 0x00, 0x01, 0x56, 0x41, 0x32, 0x00, 0x01, 0x00, 0x00, 0x00 };

std::vector<uint8_t> unknown_data { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c };

std::vector<uint8_t> end_of_frame { 0xfa, 0xfa, 0xfa, 0xfa };

std::vector<uint8_t> data_3_ch0    { 0x00, 0x33, 0x71, 0x37, 0x56, 0x4d, 0x32, 0x00, 0x4c, 0x39, 0x2f, 0x60, // hdr
                                0xe0, 0x92, 0x24, 0x02, 0xfc, 0x00, 0x00, 0x00, // hit 1
                                0xe0, 0x92, 0x34, 0x01, 0xf2, 0x00, 0x00, 0x00, // hit 2
                                0xe0, 0x92, 0x20, 0x22, 0xea, 0x00, 0x00, 0x00, // hit 3
                              };

std::vector<std::vector<uint8_t>> err_usize{err_usize1, err_usize3, err_usize4, err_usize7, err_usize12};
// clang-format on

class NMXVMM2SRSDataTest : public TestBase {

protected:
  NMXVMM2SRSData *data;
  virtual void SetUp() { data = new NMXVMM2SRSData(1125); }
  virtual void TearDown() { delete data; }
};

/** Test cases below */
TEST_F(NMXVMM2SRSDataTest, Constructor) {
  ASSERT_TRUE(data->data != nullptr);
  ASSERT_EQ(data->elems, 0U);
  ASSERT_EQ(data->error, 0U);
}

TEST_F(NMXVMM2SRSDataTest, UndersizeData) {

  for (auto v : err_usize) {
    int res = data->receive((char *)&v[0], v.size());
    ASSERT_EQ(res, 0);
    ASSERT_EQ(data->elems, 0U);
    ASSERT_EQ(data->error, v.size());
  }
}

TEST_F(NMXVMM2SRSDataTest, NoData) {
  int res = data->receive((char *)&no_data[0], no_data.size());
  ASSERT_EQ(res, 0);
  ASSERT_EQ(data->error, 0U);
  ASSERT_EQ(data->elems, 0U);
}

TEST_F(NMXVMM2SRSDataTest, UnknownData) {
  int res = data->receive((char *)&unknown_data[0], unknown_data.size());
  ASSERT_EQ(res, 0);
  ASSERT_EQ(data->error, unknown_data.size());
}

TEST_F(NMXVMM2SRSDataTest, EndOfFrame) {
  int res = data->receive((char *)&end_of_frame[0], end_of_frame.size());
  ASSERT_EQ(res, -1);
  ASSERT_EQ(data->error, 0U);
  ASSERT_EQ(data->elems, 0U);
}

TEST_F(NMXVMM2SRSDataTest, DataSizeError) {
  std::vector<unsigned int> badsizes = {13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 24,
                               25, 26, 27, 29, 30, 31, 32, 33, 34, 35};
  std::vector<unsigned int> goodsizes = {20, 28, 36};
  for (auto datasize : badsizes) {
    unsigned int res = data->receive((char *)&data_3_ch0[0], datasize);
    ASSERT_EQ(res, 0U);
    ASSERT_EQ(data->error, datasize);
    ASSERT_EQ(data->elems, 0U);
  }
  for (auto datasize : goodsizes) {
    unsigned int res = data->receive((char *)&data_3_ch0[0], datasize);
    ASSERT_EQ(res, (datasize - 12) / 8);
    ASSERT_EQ(data->error, 0U);
    ASSERT_EQ(data->elems, (datasize - 12) / 8);
  }
}

TEST_F(NMXVMM2SRSDataTest, GoodHits3) {
  int res = data->receive((char *)&data_3_ch0[0], data_3_ch0.size());
  ASSERT_EQ(res, 3);
  ASSERT_EQ(data->srshdr.fc, 0x00337137U);
  ASSERT_EQ(data->error, 0U);
  ASSERT_EQ(data->elems, 3U);
  ASSERT_EQ(data->data[0].adc, 64U);
  ASSERT_EQ(data->data[0].tdc, 73U);
  ASSERT_EQ(data->data[0].chno, 15U);
  ASSERT_EQ(data->data[0].bcid, 355U);
}

TEST_F(NMXVMM2SRSDataTest, DataOverflow) {
  auto data2 = new NMXVMM2SRSData(2);
  int res = data2->receive((char *)&data_3_ch0[0], data_3_ch0.size());
  ASSERT_EQ(res, 2);
  ASSERT_EQ(data2->error, 0U);
  ASSERT_EQ(data2->elems, 2U);
  delete data2;
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
