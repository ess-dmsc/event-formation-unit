/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/vmm2srs/EventletBuilderSRS.h>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

class EventletBuilderTest : public TestBase {
protected:
  BuilderSRS *builder;
  virtual void SetUp() {
    SRSTime time;
    SRSMappings geometry;
    geometry.define_plane(0, {{1, 0}, {1, 1}, {1, 6}, {1, 7}});
    geometry.define_plane(1, {{1, 10}, {1, 11}, {1, 14}, {1, 15}});
    builder = new BuilderSRS(time, geometry);
  }
  virtual void TearDown() { delete builder; }
};

TEST_F(EventletBuilderTest, Process) {
  Clusterer clusterer(30);
  NMXVMM2SRSData data(1125);

  //data.elems = 4;
  //auto num2 = builder->process_readout(data, clusterer);
  //ASSERT_EQ(num2, 4);
  MESSAGE() << "Bad test, working on uninitialized data\n";
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
