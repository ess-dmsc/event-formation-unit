/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <algorithm>
#include <common/DataSave.h>
#include <cstring>
#include <gdgem/NMXConfig.h>
#include <memory>
#include <test/TestBase.h>
#include <unistd.h>

std::string nocalibration{""};

using namespace Gem;

class NMXConfigTest : public TestBase {
protected:
  std::string TestJsonPath {TEST_JSON_PATH};
};

// \todo improve everything about this

/** Test cases below */
TEST_F(NMXConfigTest, ConstructorDefaults) {
  NMXConfig nmxconfig;
  EXPECT_TRUE(nmxconfig.builder_type.empty());
  EXPECT_EQ(nmxconfig.calfile, nullptr);
}

TEST_F(NMXConfigTest, EventFilter) {
  EventFilter filter;
  Event e; // use empty Event
  filter.enforce_lower_uncertainty_limit = false;
  filter.enforce_minimum_hits = false;
  EXPECT_TRUE(filter.valid(e, utpcResults()));

  filter.enforce_lower_uncertainty_limit = false;
  filter.enforce_minimum_hits = true;
  EXPECT_FALSE(filter.valid(e, utpcResults()));

  /// \todo test this behaviour
  // filter.enforce_lower_uncertainty_limit = true;
  // filter.enforce_minimum_hits = false;
  // EXPECT_FALSE(filter.valid(e));
}


TEST_F(NMXConfigTest, NoConfigFile) {
  NMXConfig nmxconfig("file_does_not_exist", nocalibration);
  EXPECT_TRUE(nmxconfig.builder_type.empty());
}

TEST_F(NMXConfigTest, DebugPrint) {
  MESSAGE() << "This is Not a test, but simply exercises the debug print code" << "\n";
  NMXConfig nmxconfig;
  nmxconfig.filter.enforce_lower_uncertainty_limit = true;
  nmxconfig.filter.enforce_minimum_hits = true;
  auto str = nmxconfig.debug();
  MESSAGE() << str << "\n";
}

TEST_F(NMXConfigTest, JsonConfig) {
  NMXConfig nmxconfig(TestJsonPath + "vmm3.json", nocalibration);
  EXPECT_EQ(100, nmxconfig.time_config.tac_slope_ns()); // Parsed from json
  EXPECT_EQ(20, nmxconfig.time_config.bc_clock_MHz());
  EXPECT_EQ(384, nmxconfig.geometry.nx());
  EXPECT_EQ(384, nmxconfig.geometry.ny());
  EXPECT_EQ(500, nmxconfig.matcher_max_delta_time);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
