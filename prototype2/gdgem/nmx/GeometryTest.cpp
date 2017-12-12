/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/nmx/Geometry.h>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

class GeometryTest : public TestBase {
protected:
  Geometry *geometry;
  virtual void SetUp() { geometry = new Geometry(); }
  virtual void TearDown() { delete geometry; }
};

TEST_F(GeometryTest, NoDimensions) {
  ASSERT_EQ(geometry->to_pixid({42}), 0U);
  ASSERT_EQ(geometry->to_pixid({42, 24}), 0U);
}

TEST_F(GeometryTest, OneDimension) {
  std::vector<uint16_t> ret {0};

  geometry->add_dimension(12);

  ASSERT_EQ(geometry->to_pixid({0}), 1);
  ASSERT_TRUE(geometry->from_pixid(1, ret));
  ASSERT_EQ(ret[0], 0);

  ASSERT_EQ(geometry->to_pixid({1}), 2);
  ASSERT_TRUE(geometry->from_pixid(2, ret));
  ASSERT_EQ(ret[0], 1);

  ASSERT_EQ(geometry->to_pixid({11}), 12);
  ASSERT_TRUE(geometry->from_pixid(12, ret));
  ASSERT_EQ(ret[0], 11);

  ASSERT_EQ(geometry->to_pixid({12}), 0);
  ASSERT_EQ(geometry->to_pixid({42, 24}), 0);

  ASSERT_FALSE(geometry->from_pixid(0, ret));
  ASSERT_FALSE(geometry->from_pixid(13, ret)); //?
}

TEST_F(GeometryTest, TwoDimensions) {
  std::vector<uint16_t> ret {0,0};

  geometry->add_dimension(5);
  geometry->add_dimension(2);
  ASSERT_EQ(geometry->to_pixid({42}), 0);
  ASSERT_EQ(geometry->to_pixid({42, 24}), 0);

  ASSERT_EQ(geometry->to_pixid({0,0}), 1);
  ASSERT_TRUE(geometry->from_pixid(1, ret));
  ASSERT_EQ(ret[0], 0);
  ASSERT_EQ(ret[1], 0);

  ASSERT_EQ(geometry->to_pixid({1,0}), 2);
  ASSERT_TRUE(geometry->from_pixid(2, ret));
  ASSERT_EQ(ret[0], 1);
  ASSERT_EQ(ret[1], 0);

  ASSERT_EQ(geometry->to_pixid({0,1}), 6);
  ASSERT_TRUE(geometry->from_pixid(6, ret));
  ASSERT_EQ(ret[0], 0);
  ASSERT_EQ(ret[1], 1);

  ASSERT_EQ(geometry->to_pixid({1,1}), 7);
  ASSERT_TRUE(geometry->from_pixid(7, ret));
  ASSERT_EQ(ret[0], 1);
  ASSERT_EQ(ret[1], 1);

  ASSERT_EQ(geometry->to_pixid({4,1}), 10);
  ASSERT_TRUE(geometry->from_pixid(10, ret));
  ASSERT_EQ(ret[0], 4);
  ASSERT_EQ(ret[1], 1);

  ASSERT_EQ(geometry->to_pixid({5,1}), 0);
  ASSERT_EQ(geometry->to_pixid({4,2}), 0);

  ASSERT_FALSE(geometry->from_pixid(11, ret)); //?
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
