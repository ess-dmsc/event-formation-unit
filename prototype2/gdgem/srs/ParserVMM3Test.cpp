/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/srs/ParserVMM3.h>
#include <gdgem/srs/ParserVMM3TestData.h>
#include <test/TestBase.h>
#include <vector>

using namespace Gem;

class VMM3SRSDataTest : public TestBase {

protected:
  VMM3SRSData *data;
  virtual void SetUp() { data = new VMM3SRSData(1125); }
  virtual void TearDown() { delete data; }

  void assertfields(unsigned int hits, unsigned int markers, unsigned int errors) {
    EXPECT_EQ(data->stats.readouts, hits);
    EXPECT_EQ(data->stats.markers, markers);
    EXPECT_EQ(data->stats.errors, errors);
  }
};

/** Test cases below */
TEST_F(VMM3SRSDataTest, Constructor) {
  EXPECT_TRUE(data->data != nullptr);
  assertfields(0, 0, 0);
  for (int i = 0; i < maximumNumberVMM*maximumNumberFECs; i++) {
    EXPECT_EQ(data->markers[i].fecTimeStamp, 0U);
  }
}

TEST_F(VMM3SRSDataTest, UndersizeData) {
  for (int dataLength = 0; dataLength <= 16; dataLength++) {
    int res = data->receive((char *)&data_3_ch0[0], dataLength);
    EXPECT_EQ(res, 0);
    assertfields(0, 0, dataLength);
    for (int i = 0; i < maximumNumberVMM*maximumNumberFECs; i++) {
      EXPECT_EQ(data->markers[i].fecTimeStamp, 0U);
    }
  }
}

TEST_F(VMM3SRSDataTest, DataOnly) {
  int res = data->receive((char *)&data_3_ch0[0], data_3_ch0.size());
  EXPECT_EQ(res, 3); // three readouts in the readout packet
  assertfields(3, 0, 0);
  for (int i = 0; i < maximumNumberVMM*maximumNumberFECs; i++) {
    EXPECT_EQ(data->markers[i].fecTimeStamp, 0U);
  }
}

TEST_F(VMM3SRSDataTest, MarkerOnly) {
  int res = data->receive((char *)&marker_3_vmm1_3[0], marker_3_vmm1_3.size());
  EXPECT_EQ(res, 0);
  assertfields(0, 3, 0);
  for (int i = 0; i < 3; i++) {
    int vmmid = i + 1; // see test data
    EXPECT_EQ(data->markers[vmmid].fecTimeStamp, i + 1);
  }
}

TEST_F(VMM3SRSDataTest, MarkerAndData) {
  int res = data->receive((char *)&marker_3_data_3[0], marker_3_data_3.size());
  EXPECT_EQ(res, 3); // three readouts in the readout packet
  assertfields(3, 3, 0);
  for (int i = 0; i < 3; i++) {
    int vmmid = i + 1; // see test data
    EXPECT_EQ(data->markers[vmmid].fecTimeStamp, i + 1);
  }
}

TEST_F(VMM3SRSDataTest, MarkerAndDataMixed) {
  int res = data->receive((char *)&marker_data_mixed_3[0], marker_data_mixed_3.size());
  EXPECT_EQ(res, 3); // three readouts in the readout packet
  assertfields(3, 3, 0);
  for (int i = 0; i < 3; i++) {
    int vmmid = i + 1; // see test data
    EXPECT_EQ(data->markers[vmmid].fecTimeStamp, i + 1);
  }
}

TEST_F(VMM3SRSDataTest, NoData) {
  int res = data->receive((char *)&no_data[0], no_data.size());
  EXPECT_EQ(res, 0);
  assertfields(0, 0, no_data.size());
}

TEST_F(VMM3SRSDataTest, InvalidDataId) {
  int res = data->receive((char *)&invalid_dataid[0], invalid_dataid.size());
  EXPECT_EQ(res, 0);
  assertfields(0, 0, invalid_dataid.size());
}

TEST_F(VMM3SRSDataTest, InconsistentDataLength) {
  int res = data->receive((char *)&inconsistent_datalen[0], inconsistent_datalen.size());
  EXPECT_EQ(res, 0);
  assertfields(0, 0, inconsistent_datalen.size());
}

TEST_F(VMM3SRSDataTest, DataLengthOverflow) {
  VMM3SRSData shortvmmbuffer(2);
  int res = shortvmmbuffer.receive((char *)& data_3_ch0[0],  data_3_ch0.size());
  EXPECT_EQ(res, 2);
  EXPECT_EQ(2, shortvmmbuffer.stats.readouts);
  EXPECT_EQ(0, shortvmmbuffer.stats.markers);
  EXPECT_EQ(6, shortvmmbuffer.stats.errors);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
