/** Copyright (C) 2016 European Spallation Source ERIC */

#include <RunSpec.h>
#include <test/TestBase.h>



class RunSpecTest : public TestBase {
protected:
};

/** Test cases below */

TEST_F(RunSpecTest, DefaultConstructor) {
  static const int start{1};
  static const int end{999};
  static const int thresh{124};
  RunSpec runspec("dir", "prefix", "postfix", start, end, "ofile", thresh);
  ASSERT_EQ(runspec.dir_, "dir");
  ASSERT_EQ(runspec.prefix_, "prefix");
  ASSERT_EQ(runspec.postfix_, "postfix");
  ASSERT_EQ(runspec.ofile_, "ofile");
  ASSERT_EQ(runspec.start_, start);
  ASSERT_EQ(runspec.end_, end);
  ASSERT_EQ(runspec.thresh_, thresh);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
