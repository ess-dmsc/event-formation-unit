/** Copyright (C) 2016 European Spallation Source ERIC */

#include "CSPECTestData.h"
#include "TestBase.h"
#include <common/MultiGridGeometry.h>
#include <cspec/CSPECChanConv.h>
#include <cspec/CSPECData.h>
#include <memory>

using namespace std;

class CspecDataTest : public TestBase {
protected:
  CSPECChanConv conv;
  MultiGridGeometry * cspecgeometry{nullptr};

  std::unique_ptr<CSPECData> dat;
  char buffer[9000];
  int size;

  virtual void SetUp() {
    cspecgeometry = new MultiGridGeometry(80, 160, 4, 16);
    dat = std::unique_ptr<CSPECData>(new CSPECData(&conv, cspecgeometry));
  }
  virtual void TearDown() {}

  void assertdatfragerr(int data, int frag, int error) {
    ASSERT_EQ(dat->elems, data);
    ASSERT_EQ(dat->frag, frag);
    ASSERT_EQ(dat->error, error);
  }
};

/** Test cases below */
TEST_F(CspecDataTest, Constructor) {
  assertdatfragerr(0, 0, 0);
  ASSERT_LT(dat->wire_thresh, 16384);
  ASSERT_LT(dat->grid_thresh, 16384);
  ASSERT_EQ(dat->datasize, 40);
}

TEST_F(CspecDataTest, ValidData) {
  for (auto v : ok) {
    size = v.size() * 4;
    dat->receive((char *)&v[0], size);
    assertdatfragerr(size / dat->datasize, 0, 0);

    for (unsigned int i = 0; i < (dat->elems); i++) {
      ASSERT_EQ(dat->data[i].module, i);
    }
  }
}

TEST_F(CspecDataTest, InvalidData) {
  for (auto v : err_pkt) {
    size = v.size() * 4;
    dat->receive((char *)&v[0], size);
    assertdatfragerr(0, 0, 1);
  }
}

TEST_F(CspecDataTest, OverUndersizeData) {
  for (auto v : err_size) {
    size = v.size() * 4;
    dat->receive((char *)&v[0], size);
    assertdatfragerr(size / dat->datasize, 1, 0);
  }
}

TEST_F(CspecDataTest, ValidateGenerator) {
  for (int i = 1; i < 225; i++) {
    size = dat->generate(buffer, 9000, i, 0, 0);
    dat->receive(buffer, size);
    assertdatfragerr(i, 0, 0);
  }
}
/** Test for oversize specification */
TEST_F(CspecDataTest, GeneratorOversize) {
  size = dat->generate(buffer, 9000, 300, 0, 0);
  dat->receive(buffer, size);
  assertdatfragerr(225, 0, 0);
}

TEST_F(CspecDataTest, InputFilterBelowThresh) {
  size = err_below_thresh.size() * 4;
  dat->receive((char *)&err_below_thresh[0], size);
  assertdatfragerr(size / dat->datasize, 0, 0);
  int discard = dat->input_filter();
  ASSERT_EQ(discard, 4);
}

TEST_F(CspecDataTest, InputFilterAboveThresh) {
  size = err_below_thresh.size() * 4;
  dat->receive((char *)&err_below_thresh[0], size);
  assertdatfragerr(size / dat->datasize, 0, 0);
  int discard = dat->input_filter();
  ASSERT_EQ(discard, 4);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
