/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/nmx/AbstractAnalyzer.h>
#include <cmath>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

using namespace Gem;

class uTPCTest : public TestBase {
protected:
  Hit hit;
  Cluster cluster;
  Event event;
  virtual void SetUp() { }
  virtual void TearDown() { }
};

// \todo tests needed

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
