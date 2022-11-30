/** Copyright (C) 2017 European Spallation Source ERIC */

#include <common/reduction/matching/GapMatcher.h>

#include <common/testutils/TestBase.h>

class GapMatcherTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(GapMatcherTest, Constructor) {
  GapMatcher matcher(125, 0, 1);
  matcher.set_minimum_time_gap(70);

  ASSERT_EQ(matcher.stats_event_count, 0);
  ASSERT_EQ(matcher.matched_events.size(), 0);

  matcher.match(true); // ever called with false?
}

TEST_F(GapMatcherTest, PrintConfig) {
  GapMatcher matcher(125, 0, 1);
  matcher.set_minimum_time_gap(70);

  GTEST_COUT << "NOT A UNIT TEST: please manually check output\n";
  GTEST_COUT << "CONFIG:\n" << matcher.config("  ");
}

// \todo do more tests

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
