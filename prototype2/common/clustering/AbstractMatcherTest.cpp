/** Copyright (C) 2017 European Spallation Source ERIC */

#include <test/TestBase.h>

#pragma GCC diagnostic push
#ifdef SYSTEM_NAME_DARWIN
#pragma GCC diagnostic ignored "-Wkeyword-macro"
#pragma GCC diagnostic ignored "-Wmacro-redefined"
#endif
#define protected public
#include <common/clustering/AbstractMatcher.h>
#ifdef protected
#undef protected
#define protected protected
#endif
#pragma GCC diagnostic pop

class MockMatcher : public AbstractMatcher {
public:
  explicit MockMatcher(uint64_t latency)
      : AbstractMatcher(latency) {}
  MockMatcher(uint64_t latency, uint8_t plane1, uint8_t plane2)
      : AbstractMatcher(latency, plane1, plane2) {}

  void match(bool) override {}
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
  EXPECT_EQ(matcher.latency_, 100);
  EXPECT_EQ(matcher.plane1_, 3);
  EXPECT_EQ(matcher.plane2_, 7);
}

TEST_F(AbstractMatcherTest, Construction2) {
  MockMatcher matcher(100);
  EXPECT_EQ(matcher.latency_, 100);
  EXPECT_EQ(matcher.plane1_, 0);
  EXPECT_EQ(matcher.plane2_, 1);
}

TEST_F(AbstractMatcherTest, InsertingMovesData) {
  MockMatcher matcher(100);

  add_cluster(x, 0, 0, 10, 1, 0, 200, 10);
  matcher.insert(0, x);

  EXPECT_TRUE(x.empty());
}

TEST_F(AbstractMatcherTest, AcceptXY) {
  MockMatcher matcher(100);

  add_cluster(x, 0, 0, 10, 1, 100, 200, 10);
  matcher.insert(0, x);

  EXPECT_EQ(matcher.unmatched_clusters_.size(), 1);
  EXPECT_EQ(matcher.latest_x_, 100);
  EXPECT_EQ(matcher.latest_y_, 0);

  add_cluster(y, 1, 0, 10, 1, 100, 200, 10);
  matcher.insert(1, y);

  EXPECT_EQ(matcher.unmatched_clusters_.size(), 2);
  EXPECT_EQ(matcher.latest_x_, 100);
  EXPECT_EQ(matcher.latest_y_, 100);
}

TEST_F(AbstractMatcherTest, AcceptOtherPlanes) {
  MockMatcher matcher(100, 3, 4);

  add_cluster(x, 3, 0, 10, 1, 100, 200, 10);
  matcher.insert(3, x);

  EXPECT_EQ(matcher.unmatched_clusters_.size(), 1);
  EXPECT_EQ(matcher.latest_x_, 100);
  EXPECT_EQ(matcher.latest_y_, 0);

  add_cluster(y, 4, 0, 10, 1, 100, 200, 10);
  matcher.insert(4, y);

  EXPECT_EQ(matcher.unmatched_clusters_.size(), 2);
  EXPECT_EQ(matcher.latest_x_, 100);
  EXPECT_EQ(matcher.latest_y_, 100);
}

TEST_F(AbstractMatcherTest, RejectUnselectedPlanes) {
  MockMatcher matcher(100, 3, 4);

  add_cluster(x, 7, 0, 10, 1, 100, 200, 10);
  matcher.insert(7, x);

  EXPECT_EQ(matcher.unmatched_clusters_.size(), 0);
  EXPECT_EQ(matcher.latest_x_, 0);
  EXPECT_EQ(matcher.latest_y_, 0);

  add_cluster(y, 0, 0, 10, 1, 100, 200, 10);
  matcher.insert(0, y);

  EXPECT_EQ(matcher.unmatched_clusters_.size(), 0);
  EXPECT_EQ(matcher.latest_x_, 0);
  EXPECT_EQ(matcher.latest_y_, 0);
}

TEST_F(AbstractMatcherTest, Ready) {
  MockMatcher matcher(100);

  Cluster c;
  c.insert({0,0,0,0});

  EXPECT_FALSE(matcher.ready_to_be_matched(c));

  matcher.latest_x_ = 99;
  matcher.latest_y_ = 99;
  EXPECT_FALSE(matcher.ready_to_be_matched(c));

  matcher.latest_x_ = 100;
  matcher.latest_y_ = 100;
  EXPECT_FALSE(matcher.ready_to_be_matched(c));

  matcher.latest_x_ = 101;
  EXPECT_FALSE(matcher.ready_to_be_matched(c));

  matcher.latest_y_ = 101;
  EXPECT_TRUE(matcher.ready_to_be_matched(c));
}

TEST_F(AbstractMatcherTest, Stash) {
  MockMatcher matcher(100);

  Event e;
  e.insert({0,0,0,0});
  e.insert({0,1,0,0});

  matcher.stash_event(e);
  EXPECT_EQ(matcher.matched_events.size(), 1);
  EXPECT_EQ(matcher.stats_event_count, 1);


  matcher.matched_events.clear();
  matcher.stash_event(e);
  EXPECT_EQ(matcher.matched_events.size(), 1);
  EXPECT_EQ(matcher.stats_event_count, 2);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
