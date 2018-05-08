/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/vmm3srs/ParserSRS.h>
#include <test/TestBase.h>
#include <vector>

// clang-format off
std::vector<uint8_t> header_only {
  0x00, 0x33, 0x71, 0x37, 0x56, 0x4d, 0x32, 0x00, 0x4c, 0x39, 0x2f, 0x60 // hdr
};

std::vector<uint8_t> data_3_ch0 {
  0x00, 0x33, 0x71, 0x37, 0x56, 0x4d, 0x32, 0x00, 0x4c, 0x39, 0x2f, 0x60, // hdr
  0xe0, 0x92, 0x24, 0x02, 0x80, 0x00, // hit 1
  0xe0, 0x92, 0x34, 0x01, 0x80, 0x00, // hit 2
  0xe0, 0x92, 0x20, 0x22, 0x80, 0x00, // hit 3
};

std::vector<uint8_t> marker_3_vmm1_3 {
  0x00, 0x33, 0x71, 0x37, 0x56, 0x4d, 0x32, 0x00, 0x4c, 0x39, 0x2f, 0x60, // hdr
  0x00, 0x00, 0x00, 0x01, 0x7d, 0x00, // marker 1: vmm1, timeStamp 1, triggerCount 999
  0x00, 0x00, 0x00, 0x02, 0x7c, 0xe1, // marker 2: vmm2, timeStamp 2, triggerCount 998
  0x00, 0x00, 0x00, 0x03, 0x7c, 0xc2, // marker 3: vmm3, timeStamp 3, triggerCount 997
};
// clang-format on

class VMM3SRSDataTest : public TestBase {

protected:
  VMM3SRSData *data;
  virtual void SetUp() { data = new VMM3SRSData(1125); }
  virtual void TearDown() { delete data; }
};

/** Test cases below */
TEST_F(VMM3SRSDataTest, Constructor) {
  ASSERT_TRUE(data->data != nullptr);
  ASSERT_EQ(data->elems, 0U);
  ASSERT_EQ(data->timet0s, 0U);
  ASSERT_EQ(data->error, 0U);
  for (int i = 0; i < 32; i++) {
    ASSERT_EQ(data->markers[i].timeStamp, 0U);
    ASSERT_EQ(data->markers[i].triggerCount, 0U);
  }
}

TEST_F(VMM3SRSDataTest, UndersizeData) {
  for (int dataLength = 0; dataLength < 12; dataLength++) {
    int res = data->receive((char *)&data_3_ch0[0], dataLength);
    ASSERT_EQ(res, 0);
    ASSERT_EQ(data->elems, 0U);
    ASSERT_EQ(data->timet0s, 0U);
    ASSERT_EQ(data->error, dataLength);
    for (int i = 0; i < 32; i++) {
      ASSERT_EQ(data->markers[i].timeStamp, 0U);
      ASSERT_EQ(data->markers[i].triggerCount, 0U);
    }
  }
}

TEST_F(VMM3SRSDataTest, HeaderOnly) {
  int dataLength = 12;
  int res = data->receive((char *)&data_3_ch0[0], dataLength);
  ASSERT_EQ(res, 0);
  ASSERT_EQ(data->elems, 0U);
  ASSERT_EQ(data->timet0s, 0U);
  ASSERT_EQ(data->error, dataLength);
  for (int i = 0; i < 32; i++) {
    ASSERT_EQ(data->markers[i].timeStamp, 0U);
    ASSERT_EQ(data->markers[i].triggerCount, 0U);
  }
}

TEST_F(VMM3SRSDataTest, DataOnly) {
  int res = data->receive((char *)&data_3_ch0[0], data_3_ch0.size());
  ASSERT_EQ(res, 3); // three hits in the readout packet
  ASSERT_EQ(data->elems, 3U);
  ASSERT_EQ(data->timet0s, 0U);
  ASSERT_EQ(data->error, 0);
  for (int i = 0; i < 32; i++) {
    ASSERT_EQ(data->markers[i].timeStamp, 0U);
    ASSERT_EQ(data->markers[i].triggerCount, 0U);
  }
}

TEST_F(VMM3SRSDataTest, MarkerOnly) {
  int res = data->receive((char *)&marker_3_vmm1_3[0], marker_3_vmm1_3.size());
  ASSERT_EQ(res, 0); // three hits in the readout packet
  ASSERT_EQ(data->elems, 0U);
  ASSERT_EQ(data->timet0s, 3U);
  ASSERT_EQ(data->error, 0);
  for (int i = 0; i < 3; i++) {
    ASSERT_EQ(data->markers[i].timeStamp, i + 1);
    ASSERT_EQ(data->markers[i].triggerCount, 1000 - i);
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
