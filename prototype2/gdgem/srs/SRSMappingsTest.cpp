/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/srs/SRSMappings.h>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

class SRSMappingsTest : public TestBase {
protected:
  SRSMappings geometry;
  Readout r;
  virtual void SetUp() {  }
  virtual void TearDown() {  }
};

TEST_F(SRSMappingsTest, GoodMapping) {
  geometry.set_mapping(0, 0, 0, 0);
  r.fec = 0;
  r.chip_id = 0;
  r.channel = 0;
  ASSERT_EQ(geometry.get_plane(r), 0);
  ASSERT_EQ(geometry.get_strip(r), 0);
}

TEST_F(SRSMappingsTest, BadMapping) {
  geometry.set_mapping(0, 16, 0, 0);
  r.fec = 0;
  r.chip_id = 0;
  r.channel = 0;
  ASSERT_EQ(geometry.get_plane(r), NMX_INVALID_PLANE_ID);
  ASSERT_EQ(geometry.get_strip(r), NMX_INVALID_GEOM_ID);
}

TEST_F(SRSMappingsTest, BadFEC) {
  geometry.set_mapping(0, 0, 0, 0);
  r.fec = 1;
  r.chip_id = 0;
  r.channel = 0;
  ASSERT_EQ(geometry.get_plane(r), NMX_INVALID_PLANE_ID);
  ASSERT_EQ(geometry.get_strip(r), NMX_INVALID_GEOM_ID);
}

TEST_F(SRSMappingsTest, BadVMM) {
  geometry.set_mapping(0, 0, 0, 0);
  r.fec = 0;
  r.channel = 0;

  r.chip_id = 15;
  ASSERT_EQ(geometry.get_plane(r), NMX_INVALID_PLANE_ID);
  ASSERT_EQ(geometry.get_strip(r), NMX_INVALID_GEOM_ID);
  r.chip_id = 16;
  ASSERT_EQ(geometry.get_plane(r), NMX_INVALID_PLANE_ID);
  ASSERT_EQ(geometry.get_strip(r), NMX_INVALID_GEOM_ID);
}

TEST_F(SRSMappingsTest, PlaneDefinition) {
  geometry.define_plane(0, {{0, 0}, {0, 1}});
  r.fec = 0;
  r.channel = 0;

  r.chip_id = 0;
  ASSERT_EQ(geometry.get_plane(r), 0);
  ASSERT_EQ(geometry.get_strip(r), 0);
  r.chip_id = 1;
  ASSERT_EQ(geometry.get_strip(r), NMX_CHIP_CHANNELS);
}

TEST_F(SRSMappingsTest, DebugString) {
  MESSAGE() << "This is not a test, just calling the debug function\n";
  geometry.define_plane(0, {{0, 0}, {0, 1}});
  auto debugstr = geometry.debug();
  MESSAGE() << debugstr << "\n";
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
