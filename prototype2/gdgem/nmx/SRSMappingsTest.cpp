/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/nmx/SRSMappings.h>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

class SRSMappingsTest : public TestBase {
protected:
  SRSMappings *geometry;
  virtual void SetUp() { geometry = new SRSMappings(); }
  virtual void TearDown() { delete geometry; }
};

TEST_F(SRSMappingsTest, GoodMapping) {
  geometry->set_mapping(0, 0, 0, 0);
  ASSERT_EQ(geometry->get_plane(0, 0), 0);
  ASSERT_EQ(geometry->get_strip(0, 0, 0), 0);
}

TEST_F(SRSMappingsTest, BadMapping) {
  geometry->set_mapping(0, 16, 0, 0);
  ASSERT_EQ(geometry->get_plane(0, 0), NMX_INVALID_GEOM_ID);
  ASSERT_EQ(geometry->get_strip(0, 0, 0), NMX_INVALID_GEOM_ID);
}

TEST_F(SRSMappingsTest, BadFEC) {
  geometry->set_mapping(0, 0, 0, 0);
  ASSERT_EQ(geometry->get_plane(1, 0), NMX_INVALID_GEOM_ID);
  ASSERT_EQ(geometry->get_strip(1, 0, 0), NMX_INVALID_GEOM_ID);
}

TEST_F(SRSMappingsTest, BadVMM) {
  geometry->set_mapping(0, 0, 0, 0);
  ASSERT_EQ(geometry->get_plane(0, 15), NMX_INVALID_GEOM_ID);
  ASSERT_EQ(geometry->get_plane(0, 16), NMX_INVALID_GEOM_ID);
  ASSERT_EQ(geometry->get_strip(0, 15, 0), NMX_INVALID_GEOM_ID);
  ASSERT_EQ(geometry->get_strip(0, 16, 0), NMX_INVALID_GEOM_ID);
}

TEST_F(SRSMappingsTest, PlaneDefinition) {
  geometry->define_plane(0, {{0, 0}, {0, 1}});
  ASSERT_EQ(geometry->get_plane(0, 0), 0);
  ASSERT_EQ(geometry->get_strip(0, 0, 0), 0);
  ASSERT_EQ(geometry->get_strip(0, 1, 0), NMX_CHIP_CHANNELS);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
