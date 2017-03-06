/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <NMX/Geometry.h>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

class GeometryTest : public TestBase {
protected:
  Geometry *geometry;
  virtual void SetUp() { geometry = new Geometry(); }
  virtual void TearDown() { delete geometry; }
};

TEST_F(GeometryTest, BadMapping) {
  ASSERT_EQ(geometry->get_plane_ID(0, 0), VMM_INVALID);
  ASSERT_EQ(geometry->get_strip_ID(0, 0, 0), VMM_INVALID);
}

TEST_F(GeometryTest, GoodMapping) {
  geometry->set_mapping(0, 0, 0, 0);
  ASSERT_EQ(geometry->get_plane_ID(0, 0), 0);
  ASSERT_EQ(geometry->get_strip_ID(0, 0, 0), 0);
}

TEST_F(GeometryTest, PlaneDefinition) {
  geometry->define_plane(0, {{0, 0}, {0, 1}});
  ASSERT_EQ(geometry->get_plane_ID(0, 0), 0);
  ASSERT_EQ(geometry->get_strip_ID(0, 0, 0), 0);
  ASSERT_EQ(geometry->get_strip_ID(0, 1, 0), VMM_TOTAL_CHANNELS);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
