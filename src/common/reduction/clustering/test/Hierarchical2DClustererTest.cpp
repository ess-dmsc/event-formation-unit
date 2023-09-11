// Copyright (C) 2023 European Spallation Source ERIC

#include <common/reduction/clustering/Hierarchical2DClusterer.h>

#include <common/testutils/TestBase.h>

class Hierarchical2DClustererTest : public TestBase {
protected:
  void mock_cluster(Hit2DVector &ret, uint16_t x_start, uint16_t x_end,
                    uint16_t x_step, uint16_t y_start, uint16_t y_end,
                    uint16_t y_step, uint64_t time_start, uint64_t time_end,
                    uint64_t time_step) {
    Hit2D e;
    e.weight = 1;
    for (e.time = time_start; e.time <= time_end; e.time += time_step)
      for (e.x_coordinate = x_start; e.x_coordinate <= x_end;
           e.x_coordinate += x_step) {
        for (e.y_coordinate = y_start; e.y_coordinate <= y_end;
             e.y_coordinate += y_step) {
          ret.push_back(e);
        }
      }
  }
};

TEST_F(Hierarchical2DClustererTest, ZeroTimeGap) {
  Hit2DVector hc;
  mock_cluster(hc, 0, 0, 1, 0, 0, 1, 1, 10, 1);

  Hierarchical2DClusterer clusterer(0, 0);
  clusterer.cluster(hc);

  EXPECT_EQ(clusterer.stats_cluster_count, 9);
  EXPECT_EQ(clusterer.clusters.size(), 9);

  clusterer.flush();
  EXPECT_EQ(clusterer.stats_cluster_count, 10);
  EXPECT_EQ(clusterer.clusters.size(), 10);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
