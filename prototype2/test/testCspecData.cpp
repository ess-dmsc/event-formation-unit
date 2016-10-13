/** Copyright (C) 2016 European Spallation Source */

#include "CSPECTestData.h"
#include <CSPECData.h>
#include <gtest/gtest.h>

using namespace std;

class CspecDataTest : public ::testing::Test {
protected:
  CSPECData *dat;

  void assertdatfragerr(int data, int frag, int error) {
    ASSERT_EQ(dat->idata, data);
    ASSERT_EQ(dat->ifrag, frag);
    ASSERT_EQ(dat->ierror, error);
  }
};

/** Test cases below */

TEST_F(CspecDataTest, ValidData) {
  for (auto v : ok) {
    printf("One iteration\n");
    int size = v.size() * 4;
    dat = new CSPECData;
    dat->receive((char *)&v[0], size);
    assertdatfragerr(size / dat->datasize, 0, 0);
    for (unsigned int i = 0; i < (dat->idata); i++) {
      ASSERT_EQ(dat->data[i].module, i);
    }
  }
}

TEST_F(CspecDataTest, InvalidData) {
  for (auto v : err_pkt) {
    dat = new CSPECData;
    int size = v.size() * 4;
    dat->receive((char *)&v[0], size);
    assertdatfragerr(0, 0, 1);
  }
}

TEST_F(CspecDataTest, OverUndersizeData) {
  for (auto v : err_size) {
    dat = new CSPECData;
    int size = v.size() * 4;
    dat->receive((char *)&v[0], size);
    assertdatfragerr(size / dat->datasize, 1, 0);
  }
}

TEST_F(CspecDataTest, ValidateGenerator) {
  char buffer[9000];
  int size;
  for (int i = 1; i < 225; i++) {
    dat = new CSPECData;
    size = dat->generate(buffer, 9000, i);
    dat->receive(buffer, size);
    assertdatfragerr(i, 0, 0);
  }

  /** Test for oversize specification */
  dat = new CSPECData;
  size = dat->generate(buffer, 9000, 300);
  dat->receive(buffer, size);
  assertdatfragerr(225, 0, 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
