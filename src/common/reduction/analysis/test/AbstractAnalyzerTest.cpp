/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/reduction/analysis/AbstractAnalyzer.h>

#include <common/testutils/TestBase.h>

class AbstractAnalyzerTest : public TestBase {
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
