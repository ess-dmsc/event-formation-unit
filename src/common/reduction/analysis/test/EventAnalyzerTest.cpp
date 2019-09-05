/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/reduction/analysis/EventAnalyzer.h>
#include <cmath>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>



class EventAnalyzerTest : public TestBase {
protected:
  Hit hit;
  Cluster cluster;
  Event event;
  virtual void SetUp() { }
  virtual void TearDown() { }
};

TEST_F(EventAnalyzerTest, AnalyzeInvalid) {
  auto result = EventAnalyzer("utpc").analyze(cluster);
  EXPECT_TRUE(std::isnan(result.center));
}

TEST_F(EventAnalyzerTest, AnalyzeAverage) {
  Hit hit;
  hit.coordinate = 0;
  hit.weight = 2;
  cluster.insert(hit);
  auto result = EventAnalyzer("utpc").analyze(cluster);
  EXPECT_EQ(result.center, 0);
  hit.coordinate = 1;
  hit.weight = 4;
  cluster.insert(hit);
  hit.coordinate = 2;
  hit.weight = 4;
  cluster.insert(hit);

  result = EventAnalyzer("utpc").analyze(cluster);
  EXPECT_EQ(cluster.hit_count(), 3);
  EXPECT_EQ(result.center, 1);

  result = EventAnalyzer("utpc_weighted").analyze(cluster);
  EXPECT_EQ(result.center, 1.2);
  EXPECT_EQ(result.center_rounded(), 1);

  cluster.clear();
  hit.time = 1;
  hit.coordinate = 0;
  hit.weight = 4;
  cluster.insert(hit);
  
  hit.time = 2;
  hit.coordinate = 2;
  hit.weight = 2;
  cluster.insert(hit);
  
  hit.time = 3;
  hit.coordinate = 3;
  hit.weight = 3;
  cluster.insert(hit);

  hit.time = 4;
  hit.coordinate = 4;
  hit.weight = 4;
  cluster.insert(hit);

  hit.time = 5;
  hit.coordinate = 5;
  hit.weight = 3;
  cluster.insert(hit);

  hit.time = 5;
  hit.coordinate = 7;
  hit.weight = 1;
  cluster.insert(hit);

  result = EventAnalyzer("center-of-mass").analyze(cluster);
  EXPECT_EQ(result.center, 3);
  EXPECT_EQ(result.time,3);
    
  result = EventAnalyzer("charge2").analyze(cluster);
  EXPECT_GT(result.center,2.74);
  EXPECT_LT(result.center,2.75);
  EXPECT_EQ(result.time, 3);
  
  result = EventAnalyzer("utpc").analyze(cluster);
  EXPECT_EQ(result.center, 6);
  EXPECT_EQ(result.time, 5);

  result = EventAnalyzer("utpc_weighted").analyze(cluster);
  EXPECT_EQ(result.center, 5.5);
  EXPECT_EQ(result.time, 5);
  

}

TEST_F(EventAnalyzerTest, AnalyzeUncert) {
  hit.weight = 1;

  hit.time = hit.coordinate = 0;
  cluster.insert(hit);
  hit.time = hit.coordinate = 1;
  cluster.insert(hit);
  hit.time = hit.coordinate = 2;
  cluster.insert(hit);

  auto result = EventAnalyzer("utpc_weighted").analyze(cluster);
  EXPECT_EQ(result.center, 2);
  EXPECT_EQ(result.uncert_lower, 1);


  hit.coordinate = 31;
  cluster.insert(hit);
  result = EventAnalyzer("utpc_weighted").analyze(cluster);
  EXPECT_EQ(result.center, 16.5);
  EXPECT_EQ(result.uncert_lower, 30);

  result = EventAnalyzer("utpc_weighted").analyze(cluster);
  EXPECT_EQ(result.center, 16.5);
  EXPECT_EQ(result.uncert_lower, 30);

  EXPECT_EQ(result.center, 16.5);
  EXPECT_EQ(result.center_rounded(), 17);
}

TEST_F(EventAnalyzerTest, AnalyzeBadY) {
  hit.weight = 1;
  event.insert(hit);
  auto result = EventAnalyzer("utpc_weighted").analyze(event);

  EXPECT_FALSE(result.good);
}

TEST_F(EventAnalyzerTest, AnalyzeBadX) {
  hit.plane = 1;
  event.insert(hit);
  auto result = EventAnalyzer("utpc_weighted").analyze(event);
  EXPECT_FALSE(result.good);
}

TEST_F(EventAnalyzerTest, AnalyzeGood) {
  hit.weight = 1;
  event.insert(hit);
  hit.plane = 1;
  event.insert(hit);
  auto result = EventAnalyzer("utpc_weighted").analyze(event);
  EXPECT_TRUE(result.good);
}

TEST_F(EventAnalyzerTest, InsertInvalid) {
  hit.weight = 1;
  hit.plane = 0;
  event.insert(hit);
  hit.plane = 1;
  event.insert(hit);
  hit.plane = 2;
  event.insert(hit);
  EXPECT_EQ(2, event.total_hit_count());
}

TEST_F(EventAnalyzerTest, DebugPrint) {
  MESSAGE() << "This is not a test, just calling debug print function\n";
  auto result = EventAnalyzer("utpc_weighted").analyze(event);
  MESSAGE() << result.to_string() << "\n";
}

// \todo more & better tests of this required

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
