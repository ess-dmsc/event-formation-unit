/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/vmm2srs/EventletBuilderSRS.h>
#include <gdgem/vmm2srs/SRSTestData.h>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

class EventletBuilderTest : public TestBase {
protected:
  BuilderSRS *builder;
  Clusterer clusterer{30};
  NMXHists hists;

  virtual void SetUp() {
    SRSTime time;
    SRSMappings geometry;
    geometry.define_plane(0, {{1, 0}, {1, 1}, {1, 6}, {1, 7}});
    geometry.define_plane(1, {{1, 10}, {1, 11}, {1, 14}, {1, 15}});
    builder = new BuilderSRS(time, geometry, "", false, false);
  }
  virtual void TearDown() { delete builder; }
};


TEST_F(EventletBuilderTest, DataTooShortForEventlets) {
  auto stats = builder->process_buffer((char*)srsdata_0_eventlets, sizeof(srsdata_0_eventlets), clusterer, hists);
  ASSERT_EQ(stats.valid_eventlets, 0);
  ASSERT_EQ(stats.geom_errors, 0);
}

TEST_F(EventletBuilderTest, InvalidGeometry) {
  auto stats = builder->process_buffer((char*)srsdata_invalid_geometry, sizeof(srsdata_invalid_geometry), clusterer, hists);
  ASSERT_EQ(stats.valid_eventlets, 1);
  ASSERT_EQ(stats.geom_errors, 1);
}

TEST_F(EventletBuilderTest, Process22Eventlets) {
  auto stats = builder->process_buffer((char*)srsdata_22_eventlets, sizeof(srsdata_22_eventlets), clusterer, hists);
  ASSERT_EQ(stats.valid_eventlets, 22);
  ASSERT_EQ(stats.geom_errors, 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
