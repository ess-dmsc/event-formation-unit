/** Copyright (C) 2017 European Spallation Source ERIC */

#include <common/clustering/GapClusterer.h>

#include <memory>
#include <stdio.h>
#include <unistd.h>
#include <test/TestBase.h>
#include <functional>

#define UNUSED __attribute__((unused))

class GapClustererTest : public TestBase {
protected:
  size_t pMinClusterSize = 3;
  // Maximum time difference between hits in time sorted cluster (x or y)
  double pMaxTimeGap = 200;
  // Maximum number of missing strips in strip sorted cluster (x or y)
  uint16_t pMaxStripGap = 2;

  virtual void SetUp() {
  }

  virtual void TearDown() {
  }
};

/// \todo Test this without sorter!!! Use presorted data that we understand


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
