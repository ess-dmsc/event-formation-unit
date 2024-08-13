// Copyright (C) 2019 European Spallation Source
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <efu/HwCheck.h>
#include <vector>

class HwCheckTest : public TestBase {
protected:
  HwCheck check;
  void SetUp() override {}
  void TearDown() override {}
};

// Test cases below
TEST_F(HwCheckTest, HwCheckFail) {
  std::vector<std::string> MandatoryInterfaces{"if_does_not_exist"};
  ASSERT_FALSE(check.checkMTU(MandatoryInterfaces, true));
}

TEST_F(HwCheckTest, HwCheckNoInterfaces) {
  std::vector<std::string> MandatoryInterfaces{};
  ASSERT_TRUE(check.checkMTU(MandatoryInterfaces, true));
}


TEST_F(HwCheckTest, HwCheckMacOs) {
  #ifdef __APPLE__
  std::vector<std::string> MandatoryInterfaces{"lo0"};
  ASSERT_TRUE(check.checkMTU(MandatoryInterfaces, true));
  #endif
}

TEST_F(HwCheckTest, HwCheckLinux) {
  #ifdef __linux__
  std::vector<std::string> MandatoryInterfaces{"lo"};
  ASSERT_TRUE(check.checkMTU(MandatoryInterfaces, true));
  #endif
}



int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
