/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/vmm2srs/EventletBuilderSRS.h>
#include <gdgem/vmm2srs/SRSTestData.h>
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
    builder = new BuilderSRS(time, geometry, "", false, false);
  }
  virtual void TearDown() { delete builder; }
};

TEST_F(EventletBuilderTest, Process) {
  Clusterer clusterer(30);
  NMXHists hists;

  auto stats = builder->process_buffer((char*)data1, sizeof(data1), clusterer, hists);
  ASSERT_EQ(stats.valid_eventlets, 22);
  ASSERT_EQ(stats.geom_errors, 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
