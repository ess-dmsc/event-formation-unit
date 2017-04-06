/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Version.h>
#include <test/TestBase.h>

class VersionTest : public TestBase {};

TEST_F(VersionTest, Constructor) {

  ASSERT_TRUE(EFU_VERSION_NUM(0,0,1) > EFU_VERSION_NUM(0,0,0));

  ASSERT_TRUE(EFU_VERSION_NUM(0,1,0) > EFU_VERSION_NUM(0,0,1));
  ASSERT_TRUE(EFU_VERSION_NUM(1,0,0) > EFU_VERSION_NUM(0,1,0));
}



int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
