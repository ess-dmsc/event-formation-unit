/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <algorithm>
#include <common/DataSave.h>
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
  ASSERT_EQ("VMM2", nmxconfig.builder_type);
  ASSERT_FALSE(nmxconfig.dump_csv);
  ASSERT_FALSE(nmxconfig.dump_h5);
}



TEST_F(NMXConfigTest, NoConfigFile) {
  NMXConfig nmxconfig("file_does_not_exist");
  ASSERT_EQ("VMM2", nmxconfig.builder_type);
  // ASSERT_EQ(256, nmxconfig.geometry_x);
  // ASSERT_EQ(256, nmxconfig.geometry_y);
  ASSERT_FALSE(nmxconfig.dump_csv);
  ASSERT_FALSE(nmxconfig.dump_h5);
}

TEST_F(NMXConfigTest, DebugPrint) {
  MESSAGE() << "This is Not a test, but simply exercises the debug print code" << "\n";
  NMXConfig nmxconfig;
  auto str = nmxconfig.debug();
  MESSAGE() << str << "\n";
}

TEST_F(NMXConfigTest, JsonConfig) {
  NMXConfig nmxconfig("../prototype2/gdgem/configs/vmm2.json");
  ASSERT_EQ(60, nmxconfig.time_config.tac_slope()); // Parsed from json
  ASSERT_EQ(20, nmxconfig.time_config.bc_clock());
}

int main(int argc, char **argv) {
    system("pwd");

    // Assume root is build/ directory - for running manually
    int ret = chdir("../build");
    if (ret != 0) {
      // Assume we're in prototype2/build/unit_tests
      ret = chdir("../../build");
      if (ret != 0) {
        printf("Unable to locate configs directory relative to:\n");
        int ret2 __attribute__((unused)) = system("pwd");
        return -1;
      }
    }
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
