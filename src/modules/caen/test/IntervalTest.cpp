// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file

#include <caen/geometry/Interval.h>
#include <common/testutils/TestBase.h>

using namespace Caen;

std::vector<std::pair<double, double>> StdInterval{
  {0.000, 0.1}, {0.101, 0.2}, {0.201, 0.3}, {0.301, 0.4},
  {0.401, 0.5}, {0.501, 0.6}, {0.601, 0.7}, {0.701, 0.8},
  {0.801, 0.9}, {0.901, 1.0}
};

class IntervalTest : public TestBase {
protected:
  bool Overlaps{false};
  double Epsilon{Interval::EPSILON};

  void SetUp() override {}
  void TearDown() override {}
};


TEST_F(IntervalTest, MiscOverlaps) {
  Overlaps = Interval::overlaps({ {0.1, 0.2}, {0.0, 0.2} });
  EXPECT_TRUE(Overlaps);

  Overlaps = Interval::overlaps({ {0.1, 0.2}, {0.2, 0.3} });
  EXPECT_TRUE(Overlaps);

  Overlaps = Interval::overlaps({ //                  overlaps 1st
    {0.0, 0.1}, {0.101, 0.2}, {0.201, 0.3}, {0.301, 0.4}, {0.02, 0.03} });
  EXPECT_TRUE(Overlaps);
}


TEST_F(IntervalTest, NoOverlaps) {
  Overlaps = Interval::overlaps(StdInterval);
  EXPECT_FALSE(Overlaps);
}


TEST_F(IntervalTest, EpsilonCheck) {
  // These should not overlap
  Overlaps = Interval::overlaps({{0.0, 0.2}, {0.2 + Epsilon, 0.3}});
  EXPECT_FALSE(Overlaps);

  // These should overlap
  Overlaps = Interval::overlaps({{0.0, 0.2}, {0.2 - Epsilon, 0.3}});
  EXPECT_TRUE(Overlaps);

  Overlaps = Interval::overlaps({{0.0, 0.2}, {0.2 + Epsilon/10, 0.3}});
  EXPECT_TRUE(Overlaps);

  Overlaps = Interval::overlaps({{0.0, 0.2}, {0.2 - Epsilon/10, 0.3}});
  EXPECT_TRUE(Overlaps);
}


TEST_F(IntervalTest, BifrostMiddleReversed) {
  Overlaps = Interval::overlaps({{0.001, 0.300}, {0.600, 0.301}, {0.601, 1.0}});
  EXPECT_FALSE(Overlaps);

  Overlaps = Interval::overlaps({{0.001, 0.300}, {0.600, 0.300}, {0.601, 1.0}});
  EXPECT_TRUE(Overlaps);

  Overlaps = Interval::overlaps({{0.001, 0.300}, {0.600, 0.300 + Epsilon}, {0.601, 1.0}});
  EXPECT_FALSE(Overlaps);

  Overlaps = Interval::overlaps({{0.001, 0.300}, {0.600, 0.300 - Epsilon}, {0.601, 1.0}});
  EXPECT_TRUE(Overlaps);

  Overlaps = Interval::overlaps({{0.001, 0.300}, {0.600, 0.300 + Epsilon/10}, {0.601, 1.0}});
  EXPECT_TRUE(Overlaps);

  Overlaps = Interval::overlaps({{0.001, 0.300}, {0.600, 0.300 - Epsilon/10}, {0.601, 1.0}});
  EXPECT_TRUE(Overlaps);
}


TEST_F(IntervalTest, NoOverlapsReversed) {
  std::vector<std::pair<double, double>> Reversed;
  for (auto & Interval : StdInterval) {
    Reversed.push_back({Interval.second, Interval.first});
  }
  Overlaps = Interval::overlaps(Reversed);
  EXPECT_FALSE(Overlaps);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
