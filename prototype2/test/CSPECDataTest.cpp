/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include "CSPECTestData.h"
#include "TestBase.h"
#include <algorithm>
#include <common/MultiGridGeometry.h>
#include <cspec/CSPECChanConv.h>
#include <cspec/CSPECData.h>
#include <memory>

using namespace std;

class CspecDataTest : public TestBase {
protected:
  CSPECChanConv conv{};

  MultiGridGeometry *cspecgeometry;

  std::unique_ptr<CSPECData> dat;
  char buffer[9000];
  int size;

  virtual void SetUp() {
    cspecgeometry = new MultiGridGeometry(1, 80, 160, 4, 16);
    dat = std::unique_ptr<CSPECData>(new CSPECData(250, &conv, cspecgeometry));
  }
  virtual void TearDown() { delete cspecgeometry; }

  void assertdatanderr(int data, int error) {
    ASSERT_EQ(dat->elems, data);
    ASSERT_EQ(dat->error, error);
  }
};

/** Test cases below */
TEST_F(CspecDataTest, Constructor) {
  assertdatanderr(0, 0);
  ASSERT_LT(dat->wire_thresh, 16384);
  ASSERT_LT(dat->grid_thresh, 16384);
  ASSERT_EQ(dat->datasize, 40);
}

TEST_F(CspecDataTest, ValidData) {
  for (auto v : ok) {
    size = v.size() * 4;
    MESSAGE() << "Size of current data: " << size << std::endl;
    dat->receive((char *)&v[0], size);
    assertdatanderr(size / dat->datasize, 0);

    for (unsigned int i = 0; i < (dat->elems); i++) {
      ASSERT_EQ(dat->data[i].module, i);
    }
  }
}

TEST_F(CspecDataTest, InvalidData) {
  for (auto v : err_pkt) {
    size = v.size() * 4;
    dat->receive((char *)&v[0], size);
    assertdatanderr(0, 40);
  }
}

TEST_F(CspecDataTest, OverUndersizeData) {
  for (auto v : err_size) {
    size = v.size() * 4;
    dat->receive((char *)&v[0], size);
    assertdatanderr(size / dat->datasize, size - dat->elems * 40);
  }
}

TEST_F(CspecDataTest, ValidateGenerator) {
  for (int i = 1; i < 225; i++) {
    size = dat->generate(buffer, 9000, i, 0, 0);
    dat->receive(buffer, size);
    assertdatanderr(i, 0);
  }
}
/** Test for oversize specification */
TEST_F(CspecDataTest, GeneratorOversize) {
  size = dat->generate(buffer, 9000, 300, 0, 0);
  dat->receive(buffer, size);
  assertdatanderr(225, 0);
}

TEST_F(CspecDataTest, InputFilterBelowThresh) {
  size = err_below_thresh.size() * 4;
  dat->receive((char *)&err_below_thresh[0], size);
  assertdatanderr(size / dat->datasize, 0);
  int discard = dat->input_filter();
  ASSERT_EQ(discard, 3);
}

TEST_F(CspecDataTest, InputFilterMisordered) {
  size = err_misorder.size() * 4;
  dat->receive((char *)&err_misorder[0], size);
  assertdatanderr(0, 40);
}

TEST_F(CspecDataTest, InputFilterNoEvents) {
  char buffer[200];
  const int databytes = 40;
  dat->receive(buffer, databytes);
  assertdatanderr(0, databytes);
  int discard = dat->input_filter();
  ASSERT_EQ(discard, 0);
}

TEST_F(CspecDataTest, CreateEventInvalidPixel) {
  char buffer[32];
  struct CSPECData::MultiGridData data;

  std::fill_n(buffer, sizeof(buffer), 0x00);
  data.module = 1;
  data.time = 0xaabbccdd;
  data.d[2] = 1;
  data.d[6] = 1;

  conv.makewirecal(0, 16383, 1);
  conv.makegridcal(0, 16383, 1);
  auto ret = dat->createevent(data, buffer);

  ASSERT_EQ(ret, -1);
}

TEST_F(CspecDataTest, CreateEventValidPixel) {
  char buffer[32];
  struct CSPECData::MultiGridData data;

  std::fill_n(buffer, sizeof(buffer), 0x00);
  data.module = 1;
  data.time = 0xaabbccdd;
  data.d[2] = 5106;
  data.d[6] = 1;

  auto ret = dat->createevent(data, buffer);

  ASSERT_EQ(ret, 0);

  ASSERT_EQ(buffer[0], (char)0xdd);
  ASSERT_EQ(buffer[1], (char)0xcc);
  ASSERT_EQ(buffer[2], (char)0xbb);
  ASSERT_EQ(buffer[3], (char)0xaa);

  EXPECT_EQ(buffer[4], (char)0x1); // pixelid 1
  EXPECT_EQ(buffer[5], (char)0x0);
  EXPECT_EQ(buffer[6], (char)0x00);
  EXPECT_EQ(buffer[7], (char)0x00);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
