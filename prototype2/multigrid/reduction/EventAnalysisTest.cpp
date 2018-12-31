/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/reduction/EventAnalysis.h>
#include <cmath>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

using namespace Multigrid;

class uTPCTest : public TestBase {
protected:
  Hit hit;
  Event event;
  EventAnalyzer analyzer;
  virtual void SetUp() {
    analyzer.mappings.add_bus(BusGeometry());
  }
  virtual void TearDown() { }
};

TEST_F(uTPCTest, AnalyzeInvalid) {
  auto result = analyzer.analyze(event);
  EXPECT_FALSE(result.good);
}

TEST_F(uTPCTest, AnalyzeBadY) {
  hit.weight = 1;
  event.insert(hit);
  auto result = analyzer.analyze(event);

  EXPECT_FALSE(result.good);
}

TEST_F(uTPCTest, AnalyzeBadX) {
  hit.plane = 1;
  event.insert(hit);
  auto result = analyzer.analyze(event);
  EXPECT_FALSE(result.good);
}

TEST_F(uTPCTest, AnalyzeGood) {
  hit.weight = 1;
  event.insert(hit);
  hit.plane = 1;
  event.insert(hit);
  auto result = analyzer.analyze(event);
  EXPECT_TRUE(result.good);
}

TEST_F(uTPCTest, InsertInvalid) {
  hit.weight = 1;
  hit.plane = 0;
  event.insert(hit);
  hit.plane = 1;
  event.insert(hit);
  hit.plane = 2;
  event.insert(hit);
  EXPECT_EQ(2, event.total_hit_count());
}

TEST_F(uTPCTest, DebugPrint) {
  MESSAGE() << "This is not a test, just calling debug print function\n";
  auto result = analyzer.analyze(event);
  MESSAGE() << result.debug() << "\n";
}

// \todo more & better tests of this required

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
