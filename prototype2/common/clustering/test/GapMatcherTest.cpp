/** Copyright (C) 2017 European Spallation Source ERIC */

#include <common/clustering/GapMatcher.h>

#include <memory>
#include <stdio.h>
#include <unistd.h>
#include <test/TestBase.h>
#include <functional>

class GapMatcherTest : public TestBase {
protected:
};

TEST_F(GapMatcherTest, Constructor) {
  GapMatcher matcher(125, 0, 1);
  matcher.set_minimum_time_gap(70);

  ASSERT_EQ(matcher.stats_event_count, 0);
  ASSERT_EQ(matcher.matched_events.size(), 0);

  matcher.match(true); // ever called with false?
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
