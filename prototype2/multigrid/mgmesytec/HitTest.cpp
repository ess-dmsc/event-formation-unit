/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/mgmesytec/Hit.h>
#include <test/TestBase.h>

using namespace Multigrid;

class HitTest : public TestBase {
protected:
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_F(HitTest, PrintsSelf) {
  Hit h;
  EXPECT_FALSE(h.debug().empty());
  // Don't really care about particular contents here
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
