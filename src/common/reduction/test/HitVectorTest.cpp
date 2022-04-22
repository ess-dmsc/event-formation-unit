// Copyright (C) 2016 - 2022 European Spallation Source, ERIC. See LICENSE file
///===--------------------------------------------------------------------===///
///
/// \file HitVectorTest.h
/// \brief Unit test for HitVector class
///
///===--------------------------------------------------------------------===///

#include <common/reduction/HitVector.h>
#include <common/testutils/TestBase.h>

#include <random>
#include <chrono>

class HitVectorTest : public TestBase {
protected:
  double center_ {0.5};
  double max_ = {80};
  double spread_ = {10};
  std::normal_distribution<double> dist;
  std::default_random_engine gen_;

  void SetUp() override {
    typedef std::chrono::system_clock myclock;
    myclock::time_point beginning = myclock::now();
    dist = std::normal_distribution<double>(center_ * max_, spread_);
    myclock::duration d = myclock::now() - beginning;
    gen_.seed(d.count());
  }
  void TearDown() override { }

  uint16_t generate_val()
  {
    return std::round(std::max(std::min(dist(gen_), double(max_)), 0.0));
  }
};

TEST_F(HitVectorTest, Visualize) {
  HitVector hits;
  for (size_t i=0; i < 10000; ++i) {
    Hit hit;
    hit.coordinate = generate_val();
    hit.weight = generate_val();
    hit.time = generate_val();
    hits.push_back(hit);
  }

//  MESSAGE() << "\n" << to_string(hits, {}) << "\n";

  MESSAGE() << "\n" << visualize(hits, {}, 100, 100) << "\n";
  MESSAGE() << "\n" << visualize(hits, {}, 30, 30) << "\n";
  MESSAGE() << "\n" << visualize(hits, {}, 0, 30) << "\n";
}

TEST_F(HitVectorTest, VisualizeEmpty) {
  HitVector hits;
  auto ret = visualize(hits, {}, 0, 30);
  ASSERT_EQ(0, ret.size());
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
