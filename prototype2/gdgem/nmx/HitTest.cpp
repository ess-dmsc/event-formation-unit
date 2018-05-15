/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/nmx/Hit.h>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

class HitTest : public TestBase {
protected:
  Hit hit;
  virtual void SetUp() {  }
  virtual void TearDown() { }
};

TEST_F(HitTest, Debug) {
  ASSERT_FALSE(hit.debug().empty());
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
