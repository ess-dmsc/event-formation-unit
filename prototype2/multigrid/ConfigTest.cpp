/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/Config.h>
#include <test/TestBase.h>

class MGConfigTest : public TestBase {
protected:
  virtual void SetUp() { }
  virtual void TearDown() { }
  std::string TestJsonPath {TEST_JSON_PATH};
};

/** Test cases below */

// \todo do proper tests

TEST_F(MGConfigTest, ConstructorDefaults) {
  Multigrid::Config mgconfig;
//  ASSERT_FALSE(mgconfig.spoof_high_time);
//  ASSERT_EQ(mgconfig.reduction_strategy, "");
//  ASSERT_EQ(mgconfig.mappings.max_z(), 0);
//  ASSERT_FALSE(mgconfig.mappings.is_valid(0, 0, 1000));
//  ASSERT_FALSE(mgconfig.mappings.is_valid(1, 0, 1000));
//  ASSERT_FALSE(mgconfig.mappings.is_valid(2, 0, 1000));
}

TEST_F(MGConfigTest, ValidConfig) {
  Multigrid::Config mgconfig(TestJsonPath + "ILL_mappings.json");
//  ASSERT_TRUE(mgconfig.spoof_high_time);
//  ASSERT_EQ(mgconfig.reduction_strategy, "maximum");
//  ASSERT_EQ(mgconfig.mappings.max_z(), 20);
//  ASSERT_TRUE(mgconfig.mappings.is_valid(0, 0, 1000));
//  ASSERT_TRUE(mgconfig.mappings.is_valid(1, 0, 1000));
//  ASSERT_TRUE(mgconfig.mappings.is_valid(2, 0, 1000));
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
