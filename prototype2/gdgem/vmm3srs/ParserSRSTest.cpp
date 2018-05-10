/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/vmm3srs/ParserSRS.h>
#include <gdgem/vmm3srs/ParserSRSTestData.h>
#include <test/TestBase.h>
#include <vector>


class VMM3SRSDataTest : public TestBase {

protected:
  VMM3SRSData *data;
  virtual void SetUp() { data = new VMM3SRSData(1125); }
  virtual void TearDown() { delete data; }

  void assertfields(unsigned int elements, unsigned int timet0s, unsigned int errors) {
    ASSERT_EQ(data->elems, elements);
    ASSERT_EQ(data->timet0s, timet0s);
    ASSERT_EQ(data->error, errors);
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
  for (int dataLength = 0; dataLength <= 12; dataLength++) {
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

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
