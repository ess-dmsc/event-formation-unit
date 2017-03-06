/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/FBSerializer.h>
#include <test/TestBase.h>

class FBSerializerTest : public TestBase {};

TEST_F(FBSerializerTest, Serialize) {

  FBSerializer fb(125000);

  uint32_t tarr[200000];
  uint32_t parr[200000];

  unsigned char *buffer = 0;
  size_t len = 0;

  auto res = fb.serialize(0x1000000020000000, 1, (char *)tarr, (char *)parr,
                          125000, &buffer, &len);
  ASSERT_EQ(len, res);
  ASSERT_TRUE(buffer != 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
