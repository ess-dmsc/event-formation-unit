/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multiblade/mbcaen/DummyClass.h>

#include <test/TestBase.h>

using namespace std;

class DummyClassTest : public TestBase {
protected:
  virtual void SetUp() {}
  virtual void TearDown() {}
};

/** Test cases below */
TEST_F(DummyClassTest, Constructor) {
  ASSERT_TRUE(1);
  ASSERT_FALSE(0);
  ASSERT_EQ(42, 42);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
