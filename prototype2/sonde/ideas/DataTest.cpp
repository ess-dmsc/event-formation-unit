/** Copyright (C) 2017 European Spallation Source ERIC */

#include <algorithm>
#include <memory>
#include <sonde/ideas/Data.h>
#include <sonde/ideas/TestData.h>
#include <test/TestBase.h>

using namespace std;

class IDEASDataTest : public TestBase {
protected:
  IDEASData * readout;
  virtual void SetUp() {
     readout = new IDEASData();
  }
  virtual void TearDown() { }
};

/** Test cases below */
TEST_F(IDEASDataTest, ErrNoData) {
    int res = readout->receive(0, 20);
    ASSERT_EQ(res, 0);
}

TEST_F(IDEASDataTest, ErrShortHeader) {
    int size = err_short_header.size() * 4;
    int res = readout->receive((char *)&err_short_header[0], size);
    ASSERT_EQ(res, 0);
}

TEST_F(IDEASDataTest, OkHeaderOnly) {
    int size = ok_header_only.size() * 4;
    int res = readout->receive((char *)&ok_header_only[0], size);
    ASSERT_EQ(res, 1);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
