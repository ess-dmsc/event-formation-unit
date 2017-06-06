/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <readout/Readout.h>
#include <readout/ReadoutTestData.h>
#include <test/TestBase.h>

class ReadoutParseTest : public TestBase {
protected:
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_F(ReadoutParseTest, InvalidData) {
  Readout readout;
  ASSERT_EQ(0, readout.receive(0,0));
  ASSERT_EQ(0, readout.receive((char*)100,-5));
  for (auto i = 0; i < 64; i++) {
    ASSERT_EQ(0, readout.receive((char*)100,i));
  }
}

TEST_F(ReadoutParseTest, BasicParsing) {
  Readout readout;
  int size = ok_one_hit.size();
  ASSERT_EQ(size, 64);
  MESSAGE() << "Size of current data: " << size << std::endl;
  auto res = readout.receive((char *)&ok_one_hit[0], size);
  ASSERT_EQ(readout.type, 1);
  ASSERT_EQ(readout.seqno, 1);
  ASSERT_EQ(readout.wordcount, 6);
  ASSERT_EQ(readout.reserved, 0);

  ASSERT_EQ(0, res);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
