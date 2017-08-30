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

TEST_F(IDEASDataTest, ErrHeaderFields) {
  int i = 0;
  for (auto v : err_hdr) {
    MESSAGE() << "err_hdr[" << i++ << "]\n";
    int res = readout->receive((char *)&v[0], v.size());
    ASSERT_EQ(res, 0);
  }
}

TEST_F(IDEASDataTest, OkHeaderOnly) {
    int size = ok_header_only.size();
    int res = readout->receive((char *)&ok_header_only[0], size);
    ASSERT_EQ(res, 0);
}

TEST_F(IDEASDataTest, OkOneEvent) {
    int size = ok_events_1.size();
    int res = readout->receive((char *)&ok_events_1[0], size);
    ASSERT_EQ(res, 1);
}

TEST_F(IDEASDataTest, OkThreeEvents) {
    int size = ok_events_3.size();
    int res = readout->receive((char *)&ok_events_3[0], size);
    ASSERT_EQ(res, 3);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
