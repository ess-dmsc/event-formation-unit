/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/reduction/Cluster.h>
#include <test/TestBase.h>

class ClusterTest : public TestBase {
protected:
  Cluster cluster;
  uint8_t invalid_plane {Hit::InvalidPlane};
};

struct CoordMass2TestData {
  uint16_t weight;    uint16_t coord;    double coordMass2;
};

static const CoordMass2TestData CoordMass2TestCases [6] = {
  { 0    , 0                  , static_cast<double>(0ULL*0ULL * 0ULL)                            },
  { 1    , 2                  , static_cast<double>(1ULL*1ULL * 2ULL)                           },
  { 11   , 22                 , static_cast<double>(11ULL*11ULL * 22ULL)                         },
  { 1111 , 2222               , static_cast<double>(1111ULL*1111ULL * 2222ULL)                   },
  { 11111, 22222              , static_cast<double>(11111ULL*11111ULL * 22222ULL)                },
  { 65535, Hit::InvalidCoord-1, static_cast<double>(65535ULL*65535ULL * (uint64_t)(Hit::InvalidCoord-1))}
};

TEST_F(ClusterTest, DefaultConstructed) {
  EXPECT_TRUE(cluster.empty());
  EXPECT_FALSE(cluster.valid());
  EXPECT_EQ(cluster.plane(), invalid_plane);
  EXPECT_EQ(cluster.hit_count(), 0);
  EXPECT_EQ(cluster.coord_span(), 0);
  EXPECT_EQ(cluster.time_span(), 0);
  EXPECT_EQ(cluster.time_gap(cluster), std::numeric_limits<uint64_t>::max());
  EXPECT_EQ(cluster.time_mass(), 0.0);
  EXPECT_EQ(cluster.time_mass2(), 0.0);
  EXPECT_EQ(cluster.coord_mass(), 0.0);
  EXPECT_EQ(cluster.weight_sum(), 0.0);
}

TEST_F(ClusterTest, TimeGapNonEmpty) {
  cluster.insert({0, 0, 0, 1});
  EXPECT_EQ(cluster.time_gap(cluster), 0);
  Cluster cluster2;
  cluster2.insert({1, 0, 0, 2});
  EXPECT_EQ(cluster.time_gap(cluster2), 1);
}

TEST_F(ClusterTest, PlaneIdentity) {
  cluster.insert({0, 0, 0, 1});
  EXPECT_FALSE(cluster.empty());
  EXPECT_TRUE(cluster.valid());
  EXPECT_EQ(cluster.plane(), 1);
}

TEST_F(ClusterTest, PlaneInvalidated) {
  cluster.insert({0, 0, 0, 1});
  cluster.insert({0, 0, 0, 2});
  EXPECT_FALSE(cluster.empty());
  EXPECT_FALSE(cluster.valid());
  EXPECT_EQ(cluster.plane(), invalid_plane);
}

TEST_F(ClusterTest, InsertRepeatedly) {
  cluster.insert({0, 0, 0, 0});
  EXPECT_EQ(cluster.hit_count(), 1);
  cluster.insert({0, 0, 0, 0});
  EXPECT_EQ(cluster.hit_count(), 2);
  cluster.insert({0, 0, 0, 0});
  EXPECT_EQ(cluster.hit_count(), 3);
}

TEST_F(ClusterTest, AdcSum) {
  cluster.insert({0, 0, 0, 0});
  EXPECT_EQ(cluster.weight_sum(), 0);
  cluster.insert({0, 0, 2, 0});
  EXPECT_EQ(cluster.weight_sum(), 2);
  cluster.insert({0, 0, 40, 0});
  EXPECT_EQ(cluster.weight_sum(), 42);
}

TEST_F(ClusterTest, Clear) {
  cluster.insert({0, 1, 0, 1});
  cluster.insert({0, 2, 0, 1});
  cluster.clear();
  EXPECT_TRUE(cluster.empty());
  EXPECT_FALSE(cluster.valid());
  EXPECT_EQ(cluster.plane(), invalid_plane);
  EXPECT_EQ(cluster.hit_count(), 0);
  EXPECT_EQ(cluster.coord_span(), 0);
  EXPECT_EQ(cluster.time_span(), 0);
  EXPECT_EQ(cluster.time_gap(cluster), std::numeric_limits<uint64_t>::max());
  EXPECT_EQ(cluster.time_mass(), 0.0);
  EXPECT_EQ(cluster.time_mass2(), 0.0);
  EXPECT_EQ(cluster.coord_mass(), 0.0);
  EXPECT_EQ(cluster.weight_sum(), 0.0);
}

TEST_F(ClusterTest, TimeSpan) {
  EXPECT_EQ(cluster.time_span(), 0);

  cluster.insert({10, 0, 0, 0});
  EXPECT_EQ(cluster.time_start(), 10);
  EXPECT_EQ(cluster.time_end(), 10);
  EXPECT_EQ(cluster.time_span(), 1);

  cluster.insert({20, 0, 0, 0});
  EXPECT_EQ(cluster.time_start(), 10);
  EXPECT_EQ(cluster.time_end(), 20);
  EXPECT_EQ(cluster.time_span(), 11);

  cluster.insert({5, 0, 0, 0});
  EXPECT_EQ(cluster.time_start(), 5);
  EXPECT_EQ(cluster.time_end(), 20);
  EXPECT_EQ(cluster.time_span(), 16);
}

TEST_F(ClusterTest, StripSpan) {
  EXPECT_EQ(cluster.coord_span(), 0);

  cluster.insert({0, 0, 0, 0});
  EXPECT_EQ(cluster.coord_start(), 0);
  EXPECT_EQ(cluster.coord_end(), 0);
  EXPECT_EQ(cluster.coord_span(), 1);

  cluster.insert({0, 10, 0, 0});
  EXPECT_EQ(cluster.coord_start(), 0);
  EXPECT_EQ(cluster.coord_end(), 10);
  EXPECT_EQ(cluster.coord_span(), 11);

  cluster.insert({0, 41, 0, 0});
  EXPECT_EQ(cluster.coord_start(), 0);
  EXPECT_EQ(cluster.coord_end(), 41);
  EXPECT_EQ(cluster.coord_span(), 42);
}

TEST_F(ClusterTest, TimeMass) {
  EXPECT_EQ(cluster.time_mass(), 0);
  EXPECT_TRUE(std::isnan(cluster.time_center()));

  cluster.insert({10, 0, 2, 0});
  EXPECT_EQ(cluster.time_mass(), 20);
  EXPECT_EQ(cluster.time_center(), 10);

  cluster.insert({0, 0, 8, 0});
  EXPECT_EQ(cluster.time_mass(), 20);
  EXPECT_EQ(cluster.time_center(), 2);
}

TEST_F(ClusterTest, CoordsMass) {
  EXPECT_EQ(cluster.coord_mass(), 0);
  EXPECT_TRUE(std::isnan(cluster.coord_center()));

  cluster.insert({0, 10, 2, 0});
  EXPECT_EQ(cluster.coord_mass(), 20);
  EXPECT_EQ(cluster.coord_center(), 10);

  cluster.insert({0, 0, 8, 0});
  EXPECT_EQ(cluster.coord_mass(), 20);
  EXPECT_EQ(cluster.coord_center(), 2);
}

TEST_F(ClusterTest, CoordMass2_Multi) {
  for (auto c : CoordMass2TestCases) {
    Cluster cluster2;
    cluster2.insert(Hit{0, c.coord, c.weight, 0});
    EXPECT_TRUE(cluster2.valid());
    EXPECT_EQ(cluster2.coord_mass2(), c.coordMass2);
  }
}

TEST_F(ClusterTest, CoordMass2_Sum) {
  double sumCoordMass2 = 0;
  for (auto c : CoordMass2TestCases) {
    cluster.insert(Hit{0, c.coord, c.weight, 0});
    sumCoordMass2 += c.coordMass2;
    EXPECT_TRUE(cluster.valid());
    EXPECT_EQ(cluster.coord_mass2(), sumCoordMass2);
  }
}

TEST_F(ClusterTest, CoordMass2) {
  cluster.insert(Hit{0, 2, 3, 0});
  EXPECT_TRUE(cluster.valid());
  EXPECT_EQ(cluster.coord_mass2(), 3*3 * 2);
}

TEST_F(ClusterTest, TimeOverlapNoOverlap) {
  Cluster cluster2;
  EXPECT_EQ(cluster.time_overlap(cluster2), 0);
  EXPECT_EQ(cluster2.time_overlap(cluster), 0);

  cluster.insert({0, 0, 0, 0});
  cluster.insert({5, 0, 0, 0});
  cluster2.insert({6, 0, 0, 0});
  cluster2.insert({12, 0, 0, 0});
  EXPECT_EQ(cluster.time_overlap(cluster2), 0);
  EXPECT_EQ(cluster2.time_overlap(cluster), 0);
}

TEST_F(ClusterTest, TimeOverlapInternalPoint) {
  Cluster cluster2;
  cluster2.insert({3, 0, 0, 0});
  cluster.insert({0, 0, 0, 0});
  cluster.insert({6, 0, 0, 0});
  EXPECT_EQ(cluster.time_overlap(cluster2), 1);
}

TEST_F(ClusterTest, TimeOverlapTouchEdge) {
  Cluster cluster2;
  cluster.insert({0, 0, 0, 0});
  cluster.insert({6, 0, 0, 0});
  cluster2.insert({6, 0, 0, 0});
  cluster2.insert({12, 0, 0, 0});
  EXPECT_EQ(cluster.time_overlap(cluster2), 1);
}

TEST_F(ClusterTest, Overlap) {
  Cluster cluster2;

  cluster.insert({0, 0, 0, 0});
  cluster.insert({7, 0, 0, 0});
  EXPECT_EQ(cluster.time_overlap(cluster2), 0);

  cluster2.insert({12, 0, 0, 0});
  EXPECT_EQ(cluster.time_overlap(cluster2), 0);

  cluster2.insert({6, 0, 0, 0});
  EXPECT_EQ(cluster.time_overlap(cluster2), 2);

  cluster2.insert({5, 0, 0, 0});
  EXPECT_EQ(cluster.time_overlap(cluster2), 3);
}

TEST_F(ClusterTest, MergeEmpty) {
  cluster.insert({0, 0, 0, 0});
  cluster.insert({0, 0, 0, 0});
  cluster.insert({0, 0, 0, 0});

  Cluster cluster2;
  cluster.merge(cluster2);

  EXPECT_EQ(cluster.hit_count(), 3);
  EXPECT_EQ(cluster2.hit_count(), 0);
}

TEST_F(ClusterTest, MergeToEmpty) {
  Cluster cluster2;
  cluster2.insert({0, 0, 0, 0});
  cluster2.insert({0, 0, 0, 0});
  cluster2.insert({0, 0, 0, 0});

  cluster.merge(cluster2);

  EXPECT_EQ(cluster.hit_count(), 3);
  EXPECT_EQ(cluster.plane(), 0);
  EXPECT_TRUE(cluster.valid());
}

TEST_F(ClusterTest, Merge2Valid) {
  cluster.insert({0, 5, 1, 0});
  cluster.insert({7, 5, 1, 0});

  Cluster cluster2;
  cluster2.insert({12, 15, 1, 0});
  cluster2.insert({6, 15, 1, 0});
  cluster2.insert({5, 15, 1, 0});

  cluster.merge(cluster2);

  EXPECT_EQ(cluster.hit_count(), 5);
  EXPECT_EQ(cluster.time_span(), 13);
  EXPECT_EQ(cluster.coord_span(), 11);
  EXPECT_EQ(cluster.weight_sum(), 5);
  EXPECT_EQ(cluster.plane(), 0);
  EXPECT_TRUE(cluster.valid());

  EXPECT_EQ(cluster2.hit_count(), 0);
  EXPECT_FALSE(cluster2.valid());
}

TEST_F(ClusterTest, MergeMismatchedPlanes) {
  cluster.insert({0, 5, 1, 0});
  cluster.insert({7, 5, 1, 0});

  Cluster cluster2;
  cluster2.insert({12, 15, 1, 1});
  cluster2.insert({6, 15, 1, 1});
  cluster2.insert({5, 15, 1, 1});

  cluster.merge(cluster2);

  EXPECT_EQ(cluster.hit_count(), 5);
  EXPECT_EQ(cluster.time_span(), 13);
  EXPECT_EQ(cluster.coord_span(), 11);
  EXPECT_EQ(cluster.weight_sum(), 5);
  EXPECT_EQ(cluster.plane(), invalid_plane);
  EXPECT_FALSE(cluster.valid());

  EXPECT_EQ(cluster2.hit_count(), 0);
  EXPECT_FALSE(cluster2.valid());
}

TEST_F(ClusterTest, PrintDebug) {
  cluster.insert({0, 5, 1, 0});
  cluster.insert({7, 5, 1, 0});

  MESSAGE() << "NOT A UNIT TEST: please manually check output\n";
  MESSAGE() << "SIMPLE:\n" << cluster.to_string("  ", false) << "\n";
  MESSAGE() << "VERBOSE:\n" << cluster.to_string("  ", true);
  MESSAGE() << "VISUALIZE:\n" << cluster.visualize("  ");
}

// \todo have functions for generation of randomized clusters

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
