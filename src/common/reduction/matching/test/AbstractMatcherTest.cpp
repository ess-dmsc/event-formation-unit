/** Copyright (C) 2017 European Spallation Source ERIC */

#include <common/testutils/TestBase.h>
#include <common/reduction/matching/AbstractMatcher.h>

class MockMatcher : public AbstractMatcher {
public:
  using AbstractMatcher::AbstractMatcher;

  void match(bool) override {}
  using AbstractMatcher::maximum_latency_;
  using AbstractMatcher::PlaneA;
  using AbstractMatcher::PlaneB;
  using AbstractMatcher::LatestA;
  using AbstractMatcher::LatestB;
  using AbstractMatcher::unmatched_clusters_;
  using AbstractMatcher::ready_to_be_matched;
  using AbstractMatcher::stash_event;
};

class AbstractMatcherTest : public TestBase {
protected:
  ClusterContainer x, y;

  void add_cluster(ClusterContainer &ret, uint8_t plane,
                   uint16_t coord_start, uint16_t coord_end, uint16_t coord_step,
                   uint64_t time_start, uint64_t time_end, uint64_t time_step) {
    Cluster c;
    Hit e;
    e.plane = plane;
    e.weight = 1;
    for (e.time = time_start; e.time <= time_end; e.time += time_step)
      for (e.coordinate = coord_start; e.coordinate <= coord_end; e.coordinate += coord_step)
        c.insert(e);
    ret.push_back(c);
  }
};

TEST_F(AbstractMatcherTest, Construction1) {
  MockMatcher matcher(100, 3, 7);
  EXPECT_EQ(matcher.unmatched_clusters_.size(), 0);
  EXPECT_EQ(matcher.maximum_latency_, 100);
  EXPECT_EQ(matcher.PlaneA, 3);
  EXPECT_EQ(matcher.PlaneB, 7);
}

TEST_F(AbstractMatcherTest, InsertingMovesData) {
  MockMatcher matcher(100, 0, 1);

  add_cluster(x, 0, 0, 10, 1, 0, 200, 10);
  matcher.insert(0, x);

  EXPECT_TRUE(x.empty());
}


TEST_F(AbstractMatcherTest, AcceptXY) {
  MockMatcher matcher(100, 0, 1);

  add_cluster(x, matcher.PlaneA, 0, 10, 1, 100, 200, 10);
  matcher.insert(0, x);

  EXPECT_EQ(matcher.unmatched_clusters_.size(), 1);
  EXPECT_EQ(matcher.LatestA, 100);
  EXPECT_EQ(matcher.LatestB, 0);

  add_cluster(y, matcher.PlaneB, 0, 10, 1, 100, 200, 10);
  matcher.insert(1, y);

  EXPECT_EQ(matcher.unmatched_clusters_.size(), 2);
  EXPECT_EQ(matcher.LatestA, 100);
  EXPECT_EQ(matcher.LatestB, 100);
}

// Differs
TEST_F(AbstractMatcherTest, AcceptXYImplicitPlane) {
  MockMatcher matcher(100, 0, 1);

  add_cluster(x, matcher.PlaneA, 0, 10, 1, 100, 200, 10);
  matcher.insert(x);

  EXPECT_EQ(matcher.unmatched_clusters_.size(), 1);
  EXPECT_EQ(matcher.LatestA, 100);
  EXPECT_EQ(matcher.LatestB, 0);

  add_cluster(y, matcher.PlaneB, 0, 10, 1, 100, 200, 10);
  matcher.insert(y);

  EXPECT_EQ(matcher.unmatched_clusters_.size(), 2);
  EXPECT_EQ(matcher.LatestA, 100);
  EXPECT_EQ(matcher.LatestB, 100);
}

TEST_F(AbstractMatcherTest, AddClustersInvalidPlane) {
  MockMatcher matcher(100, 0, 1);
  const uint8_t InvalidPlane{8};

  EXPECT_EQ(matcher.stats_rejected_clusters, 0);

  add_cluster(x, InvalidPlane, 0, 10, 1, 100, 200, 10);
  matcher.insert(x);

  EXPECT_EQ(matcher.stats_rejected_clusters, 1);
}

TEST_F(AbstractMatcherTest, AcceptOtherPlanes) {
  MockMatcher matcher(100, 3, 4);

  add_cluster(x, 3, 0, 10, 1, 100, 200, 10);
  matcher.insert(3, x);

  EXPECT_EQ(matcher.unmatched_clusters_.size(), 1);
  EXPECT_EQ(matcher.LatestA, 100);
  EXPECT_EQ(matcher.LatestB, 0);

  add_cluster(y, 4, 0, 10, 1, 100, 200, 10);
  matcher.insert(4, y);

  EXPECT_EQ(matcher.unmatched_clusters_.size(), 2);
  EXPECT_EQ(matcher.LatestA, 100);
  EXPECT_EQ(matcher.LatestB, 100);
}

TEST_F(AbstractMatcherTest, RejectUnselectedPlanes) {
  MockMatcher matcher(100, 3, 4);

  add_cluster(x, 7, 0, 10, 1, 100, 200, 10);
  matcher.insert(7, x);

  EXPECT_EQ(matcher.unmatched_clusters_.size(), 0);
  EXPECT_EQ(matcher.LatestA, 0);
  EXPECT_EQ(matcher.LatestB, 0);

  add_cluster(y, 0, 0, 10, 1, 100, 200, 10);
  matcher.insert(0, y);

  EXPECT_EQ(matcher.unmatched_clusters_.size(), 0);
  EXPECT_EQ(matcher.LatestA, 0);
  EXPECT_EQ(matcher.LatestB, 0);
}

TEST_F(AbstractMatcherTest, Ready) {
  MockMatcher matcher(100, 0, 1);

  Cluster c;
  c.insert({0, 0, 0, 0});

  EXPECT_FALSE(matcher.ready_to_be_matched(c));

  matcher.LatestA = 99;
  matcher.LatestB = 99;
  EXPECT_FALSE(matcher.ready_to_be_matched(c));

  matcher.LatestA = 100;
  matcher.LatestB = 100;
  EXPECT_FALSE(matcher.ready_to_be_matched(c));

  matcher.LatestA = 101;
  EXPECT_FALSE(matcher.ready_to_be_matched(c));

  matcher.LatestB = 101;
  EXPECT_TRUE(matcher.ready_to_be_matched(c));
}

TEST_F(AbstractMatcherTest, Stash) {
  MockMatcher matcher(100, 0, 1);

  Event e;
  e.insert({0, 0, 0, 0});
  e.insert({0, 1, 0, 0});

  matcher.stash_event(e);
  EXPECT_EQ(matcher.matched_events.size(), 1);
  EXPECT_EQ(matcher.stats_event_count, 1);

  matcher.matched_events.clear();
  matcher.stash_event(e);
  EXPECT_EQ(matcher.matched_events.size(), 1);
  EXPECT_EQ(matcher.stats_event_count, 2);
}

TEST_F(AbstractMatcherTest, PrintConfigStatus) {
  MockMatcher matcher(100, 3, 4);

  add_cluster(x, 3, 0, 3, 1,
              100, 120, 10);
  matcher.insert(3, x);

  add_cluster(y, 4, 0, 3, 1,
              100, 120, 10);
  matcher.insert(4, y);

  MESSAGE() << "NOT A UNIT TEST: please manually check output\n";
  MESSAGE() << "CONFIG:\n" << matcher.config("  ");
  MESSAGE() << "SIMPLE STATUS:\n" << matcher.status("  ", false);
  MESSAGE() << "VERBOSE STATUS:\n" << matcher.status("  ", true);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
