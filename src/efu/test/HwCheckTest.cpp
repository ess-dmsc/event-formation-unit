/** Copyright (C) 2019 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <efu/HwCheck.h>
#include <common/testutils/TestBase.h>
#include <vector>

class HwCheckTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

/** Test cases below */
TEST_F(HwCheckTest, HwCheckPass) {
  std::vector<std::string> IgnoredInterfaces {"0", "00", "br-"};
  HwCheck check;
  bool pass = check.checkMTU(IgnoredInterfaces, true);
  ASSERT_TRUE(pass);
}

// Can't get this to fail on all platforms on Jenkins
// TEST_F(HwCheckTest, HwCheckFail) {
//   std::vector<std::string> IgnoredInterfaces {"0", "00"};
//   HwCheck check;
//   check.setMinimumMTU(6553500);
//   bool pass = check.checkMTU(IgnoredInterfaces);
//   ASSERT_FALSE(pass);
// }

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
