/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/analysis/AbstractAnalyzer.h>
#include <cmath>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

class ReducedEventTest : public TestBase {
protected:
  Hit hit;
  Cluster cluster;
  Event event;
  void SetUp() override { }
  void TearDown() override { }
};

// \todo tests needed

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
