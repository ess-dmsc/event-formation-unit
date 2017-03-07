/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <nmxvmm2srs/EventletBuilder.h>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

class EventletBuilderTest : public TestBase {
protected:
  EventletBuilder * builder;
  virtual void SetUp() { 
    Time time;
    Geometry geometry;
    builder = new EventletBuilder(time, geometry);
  }
  virtual void TearDown() { delete builder; }
};

TEST_F(EventletBuilderTest, Process) {
  Clusterer clusterer(30);
  NMXVMM2SRSData data(1125);

  auto num1 = builder->process_readout(data, clusterer);
  ASSERT_EQ(num1, 0);

  data.elems = 4;
  auto num2 = builder->process_readout(data, clusterer);
  ASSERT_EQ(num2, 4);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
