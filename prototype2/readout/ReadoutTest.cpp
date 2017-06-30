/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <readout/Readout.h>
#include <readout/ReadoutTestData.h>
#include <test/TestBase.h>

class ReadoutTest : public TestBase {
protected:
  char buffer[9000];
  virtual void SetUp() {
    memset(buffer, 0, sizeof(buffer));
  }
  virtual void TearDown() {}
};

TEST_F(ReadoutTest, InvalidBuffer) {
  Readout readout;
  ASSERT_EQ(readout.validate(0,0), -Readout::EBUFFER);
}

TEST_F(ReadoutTest, InvalidDataSize) {
  Readout readout;
  ASSERT_EQ(readout.validate(buffer,-5), -Readout::ESIZE);
  for (auto i = 0; i < 64; i++) {
    ASSERT_EQ(readout.validate(buffer,i), -Readout::ESIZE);
  }
}

TEST_F(ReadoutTest, CheckPadding) {
  Readout readout;
  for (auto size = 65; size < 8960; size++) {
    if (size % 64 != 0) {
      ASSERT_EQ(readout.validate(buffer, size), -Readout::EPAD);
    } else {
      ASSERT_EQ(readout.validate(buffer, size), Readout::OK);
    }
  }
}

TEST_F(ReadoutTest, BasicParsing) {
  Readout readout;
  int size = ok_one_hit.size();
  ASSERT_EQ(size, 64);

  auto res = readout.validate((char *)&ok_one_hit[0], size);
  ASSERT_EQ(res, 0);

  ASSERT_EQ(readout.type, 1);
  ASSERT_EQ(readout.seqno, 1);
  ASSERT_EQ(readout.wordcount, 6);
  ASSERT_EQ(readout.reserved, 0);
}

TEST_F(ReadoutTest, PktAndHeaderSizeMismatch) {
  Readout readout;
  int size = err_size_mismatch.size();
  ASSERT_EQ(size, 64);

  auto res = readout.validate((char *)&err_size_mismatch[0], size);
  ASSERT_EQ(res, -Readout::EHDR);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
