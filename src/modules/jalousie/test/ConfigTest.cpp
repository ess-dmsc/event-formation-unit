/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <jalousie/Config.h>
#include <test/TestBase.h>

class JalConfigTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
  std::string TestJsonPath{TEST_DATA_PATH};
};

/** Test cases below */

// \todo do proper tests

TEST_F(JalConfigTest, ConstructorDefaults) {
  Jalousie::Config config;
  MESSAGE() << config.debug();
  //  ASSERT_FALSE(config.spoof_high_time);
  //  ASSERT_EQ(config.reduction_strategy, "");
  //  ASSERT_EQ(config.mappings.max_z(), 0);
  //  ASSERT_FALSE(config.mappings.is_valid(0, 0, 1000));
  //  ASSERT_FALSE(config.mappings.is_valid(1, 0, 1000));
  //  ASSERT_FALSE(config.mappings.is_valid(2, 0, 1000));
}

TEST_F(JalConfigTest, ValidConfig) {
  Jalousie::Config config(TestJsonPath + "v20_mappings.json");
  MESSAGE() << config.debug();
  //  ASSERT_TRUE(config.spoof_high_time);
  //  ASSERT_EQ(config.reduction_strategy, "maximum");
  //  ASSERT_EQ(config.mappings.max_z(), 20);
  //  ASSERT_TRUE(config.mappings.is_valid(0, 0, 1000));
  //  ASSERT_TRUE(config.mappings.is_valid(1, 0, 1000));
  //  ASSERT_TRUE(config.mappings.is_valid(2, 0, 1000));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
