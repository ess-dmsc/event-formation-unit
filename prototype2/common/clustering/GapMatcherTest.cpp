/** Copyright (C) 2017 European Spallation Source ERIC */

#include <common/clustering/GapMatcher.h>

#include <memory>
#include <stdio.h>
#include <unistd.h>
#include <test/TestBase.h>
#include <functional>

std::vector<Hit> tg126_p0 = {
  {0,   15, 1000, 0},
  {126, 15, 1000, 0}
};

std::vector<Hit> tg126_p1 = {
  {0,   15, 1000, 1},
  {126, 15, 1000, 1}
};

class GapMatcherTest : public TestBase {
protected:
    Cluster c0, c1;
    ClusterContainer cc;
};

TEST_F(GapMatcherTest, NoTimeOverlap) {
  GapMatcher matcher(125, 0);

  for (auto & hit  : tg126_p0) {
    c0.insert(hit);
  }
  cc.push_back(c0);
  matcher.insert(0, cc);

  ASSERT_EQ(matcher.stats_event_count, 0);
  ASSERT_EQ(matcher.matched_events.size(), 0);

  matcher.match(true); // ever called with false?
  ASSERT_EQ(matcher.stats_event_count, 0);
  ASSERT_EQ(matcher.matched_events.size(), 0);

  cc.clear();
  for (auto & hit : tg126_p1) {
    c1.insert(hit);
  }
  cc.push_back(c1);
  matcher.insert(1, cc);

  matcher.match(true); // ever called with false?
  ASSERT_EQ(matcher.stats_event_count, 0);
  ASSERT_EQ(matcher.matched_events.size(), 0);

}



int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
