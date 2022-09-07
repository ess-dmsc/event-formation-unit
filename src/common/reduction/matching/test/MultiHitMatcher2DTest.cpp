/** Copyright (C) 2017 European Spallation Source ERIC */

#include <common/reduction/matching/MultiHitMatcher2D.h>

#include <common/testutils/TestBase.h>

class MultiHitMatcher2DTest : public TestBase {
protected:
  ClusterContainer x, y;

  void SetUp() override {}
  void TearDown() override {}

  void add_cluster(ClusterContainer &ret, uint8_t plane, uint16_t coord_start,
                   uint16_t coord_end, uint16_t coord_step, uint64_t time_start,
                   uint64_t time_end, uint64_t time_step) {
    Cluster c;
    Hit e;
    e.plane = plane;
    e.weight = 1;
    for (e.time = time_start; e.time <= time_end; e.time += time_step)
      for (e.coordinate = coord_start; e.coordinate <= coord_end;
           e.coordinate += coord_step)
        c.insert(e);
    ret.push_back(c);
  }

  void add_multi_hit_cluster(ClusterContainer &ret, uint8_t plane, uint16_t weight_a, uint16_t coord_start_a,
                   uint16_t coord_end_a, uint16_t coord_step_a, uint16_t weight_b, uint16_t coord_start_b,
                   uint16_t coord_end_b, uint16_t coord_step_b, uint64_t time_start,
                   uint64_t time_end, uint64_t time_step) {
    Cluster c;
    Hit e;
    e.plane = plane;
    
    for (e.time = time_start; e.time <= time_end; e.time += time_step){
      e.weight = weight_a;
      for (e.coordinate = coord_start_a; e.coordinate <= coord_end_a;
           e.coordinate += coord_step_a)
        c.insert(e);
      e.weight = weight_b;
      for (e.coordinate = coord_start_b; e.coordinate <= coord_end_b;
          e.coordinate += coord_step_b)
        c.insert(e);
    }
    ret.push_back(c);
  }
};

TEST_F(MultiHitMatcher2DTest, Constructor) {
  MultiHitMatcher2D matcher(125, 0, 1);
  matcher.set_minimum_time_gap(70);

  ASSERT_EQ(matcher.stats_event_count, 0);
  ASSERT_EQ(matcher.matched_events.size(), 0);

  matcher.match(true); // ever called with false?
}

TEST_F(MultiHitMatcher2DTest, PrintConfig) {
  MultiHitMatcher2D matcher(125, 0, 1);
  matcher.set_minimum_time_gap(70);

  MESSAGE() << "NOT A UNIT TEST: please manually check output\n";
  MESSAGE() << "CONFIG:\n" << matcher.config("  ");
}

TEST_F(MultiHitMatcher2DTest, MatchSingleEvent){
  MultiHitMatcher2D matcher(125, 0, 1);
  matcher.set_minimum_time_gap(70);

  ASSERT_EQ(matcher.stats_event_count, 0);
  ASSERT_EQ(matcher.matched_events.size(), 0);

  add_cluster(x, 0, 50, 55, 1, 0, 5, 1);
  add_cluster(y, 1, 100, 110, 2, 0, 5, 1);

  matcher.insert(0, x);
  matcher.insert(1, y);

  matcher.match(true);

  ASSERT_EQ(matcher.stats_event_count, 1);

}

TEST_F(MultiHitMatcher2DTest, MatchMultiEvent){
  MultiHitMatcher2D matcher(125, 0, 1);
  matcher.set_minimum_time_gap(70);

  ASSERT_EQ(matcher.stats_event_count, 0);
  ASSERT_EQ(matcher.matched_events.size(), 0);

  add_multi_hit_cluster(x, 0, 10, 50, 55, 1, 100, 200, 205, 1, 0, 5, 1);
  add_multi_hit_cluster(y, 1, 10, 100, 110, 2, 100, 10, 15, 1, 0, 5, 1);

  matcher.insert(0, x);
  matcher.insert(1, y);

  matcher.match(true);

  ASSERT_EQ(matcher.stats_event_count, 2);

}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
