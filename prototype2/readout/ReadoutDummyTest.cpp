/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <readout/ReadoutDummy.h>
#include <readout/ReadoutTestData.h>
#include <test/TestBase.h>

#define DETECTOR_TYPE 1

class ReadoutDummyTest : public TestBase {
protected:
  char buffer[9000];
  virtual void SetUp() {
    memset(buffer, 0, sizeof(buffer));
  }
  virtual void TearDown() {}
};


TEST_F(ReadoutDummyTest, Parser) {
  ReadoutDummy readout(DETECTOR_TYPE);

  int size = ok_one_hit.size();

  auto res = readout.parse((char *)&ok_one_hit[0], size);
  ASSERT_EQ(res, 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
