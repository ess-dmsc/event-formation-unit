/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/vmm3/ParserVMM3.h>
#include <gdgem/vmm3/ParserVMM3TestData.h>
#include <test/TestBase.h>
#include <vector>


class VMM3SRSDataTest : public TestBase {

protected:
  VMM3SRSData *data;
  virtual void SetUp() { data = new VMM3SRSData(1125); }
  virtual void TearDown() { delete data; }

  void assertfields(unsigned int hits, unsigned int timet0s, unsigned int errors) {
    ASSERT_EQ(data->stats.hits, hits);
    ASSERT_EQ(data->stats.timet0s, timet0s);
    ASSERT_EQ(data->stats.errors, errors);
  }
};

/** Test cases below */
TEST_F(VMM3SRSDataTest, Constructor) {
  ASSERT_TRUE(data->data != nullptr);
  assertfields(0, 0, 0);
  for (int i = 0; i < 32; i++) {
    ASSERT_EQ(data->markers[i].timeStamp, 0U);
    ASSERT_EQ(data->markers[i].triggerCount, 0U);
  }
}

TEST_F(VMM3SRSDataTest, UndersizeData) {
  for (int dataLength = 0; dataLength <= 16; dataLength++) {
    int res = data->receive((char *)&data_3_ch0[0], dataLength);
    ASSERT_EQ(res, 0);
    assertfields(0, 0, dataLength);
    for (int i = 0; i < 32; i++) {
      ASSERT_EQ(data->markers[i].timeStamp, 0U);
      ASSERT_EQ(data->markers[i].triggerCount, 0U);
    }
  }
}

TEST_F(VMM3SRSDataTest, DataOnly) {
  int res = data->receive((char *)&data_3_ch0[0], data_3_ch0.size());
  ASSERT_EQ(res, 3); // three hits in the readout packet
  assertfields(3, 0, 0);
  for (int i = 0; i < 32; i++) {
    ASSERT_EQ(data->markers[i].timeStamp, 0U);
    ASSERT_EQ(data->markers[i].triggerCount, 0U);
  }
}

TEST_F(VMM3SRSDataTest, MarkerOnly) {
  int res = data->receive((char *)&marker_3_vmm1_3[0], marker_3_vmm1_3.size());
  ASSERT_EQ(res, 0);
  assertfields(0, 3, 0);
  for (int i = 0; i < 3; i++) {
    ASSERT_EQ(data->markers[i].timeStamp, i + 1);
    ASSERT_EQ(data->markers[i].triggerCount, 1000 - i);
  }
}

TEST_F(VMM3SRSDataTest, MarkerAndData) {
  int res = data->receive((char *)&marker_3_data_3[0], marker_3_data_3.size());
  ASSERT_EQ(res, 3); // three hits in the readout packet
  assertfields(3, 3, 0);
  for (int i = 0; i < 3; i++) {
    ASSERT_EQ(data->markers[i].timeStamp, i + 1);
    ASSERT_EQ(data->markers[i].triggerCount, 1000 - i);
  }
}

TEST_F(VMM3SRSDataTest, MarkerAndDataMixed) {
  int res = data->receive((char *)&marker_data_mixed_3[0], marker_data_mixed_3.size());
  ASSERT_EQ(res, 3); // three hits in the readout packet
  assertfields(3, 3, 0);
  for (int i = 0; i < 3; i++) {
    ASSERT_EQ(data->markers[i].timeStamp, i + 1);
    ASSERT_EQ(data->markers[i].triggerCount, 1000 - i);
  }
}

TEST_F(VMM3SRSDataTest, NoData) {
  int res = data->receive((char *)&no_data[0], no_data.size());
  ASSERT_EQ(res, 0);
  assertfields(0, 0, no_data.size());
}

TEST_F(VMM3SRSDataTest, InvalidDataId) {
  int res = data->receive((char *)&invalid_dataid[0], invalid_dataid.size());
  ASSERT_EQ(res, 0);
  assertfields(0, 0, invalid_dataid.size());
}

TEST_F(VMM3SRSDataTest, InconsistentDataLength) {
  int res = data->receive((char *)&inconsistent_datalen[0], inconsistent_datalen.size());
  ASSERT_EQ(res, 0);
  assertfields(0, 0, inconsistent_datalen.size());
}

TEST_F(VMM3SRSDataTest, DataLengthOverflow) {
  VMM3SRSData shortvmmbuffer(2);
  int res = shortvmmbuffer.receive((char *)& data_3_ch0[0],  data_3_ch0.size());
  ASSERT_EQ(res, 2);
  ASSERT_EQ(2, shortvmmbuffer.stats.hits);
  ASSERT_EQ(0, shortvmmbuffer.stats.timet0s);
  ASSERT_EQ(6, shortvmmbuffer.stats.errors);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
