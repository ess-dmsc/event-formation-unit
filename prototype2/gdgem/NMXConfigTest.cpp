/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <algorithm>
#include <cstring>
#include <gdgem/NMXConfig.h>
#include <memory>
#include <test/TestBase.h>

class NMXConfigTest : public TestBase {
protected:
  virtual void SetUp() {
  }

  virtual void TearDown() { }
};

/** Test cases below */
TEST_F(NMXConfigTest, ConstructorDefaults) {
  NMXConfig nmxconfig;
  ASSERT_EQ("SRS", nmxconfig.builder_type);
  ASSERT_EQ(256, nmxconfig.geometry_x);
  ASSERT_EQ(256, nmxconfig.geometry_y);
  ASSERT_FALSE(nmxconfig.dump_csv);
  ASSERT_FALSE(nmxconfig.dump_h5);
}


TEST_F(NMXConfigTest, NoConfigFile) {
  NMXConfig nmxconfig("file_does_not_exist");
  ASSERT_EQ("SRS", nmxconfig.builder_type);
  ASSERT_EQ(256, nmxconfig.geometry_x);
  ASSERT_EQ(256, nmxconfig.geometry_y);
  ASSERT_FALSE(nmxconfig.dump_csv);
  ASSERT_FALSE(nmxconfig.dump_h5);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
