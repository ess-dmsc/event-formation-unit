/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/reduction/EventProcessingStats.h>
#include <test/TestBase.h>

using namespace Multigrid;

class EventProcessingStatsTest : public TestBase {
protected:
  void SetUp() override {
  }
  void TearDown() override {
  }
};

TEST_F(EventProcessingStatsTest, blah) {
  EventProcessingStats s;
  s.clear();
  // \todo actual tests
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
