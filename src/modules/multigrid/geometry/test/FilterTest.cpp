/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/geometry/Filter.h>
#include <common/testutils/TestBase.h>

using namespace Multigrid;

class FilterTest : public TestBase {
protected:
  Filter f;
  FilterSet fs;
  void SetUp() override {
  }
  void TearDown() override {
  }
};


TEST_F(FilterTest, DefaultConstructedValues) {
  EXPECT_EQ(f.minimum, 0);
  EXPECT_EQ(f.maximum, std::numeric_limits<uint16_t>::max());
  EXPECT_EQ(f.rescale_factor, 1.0);
  EXPECT_TRUE(f.trivial());
}

TEST_F(FilterTest, RescaleWorks) {
  f.rescale_factor = 0.5;
  EXPECT_EQ(f.rescale(2), 1);
  EXPECT_EQ(f.rescale(10), 5);

  f.rescale_factor = 3;
  EXPECT_EQ(f.rescale(2), 6);
  EXPECT_EQ(f.rescale(10), 30);
}

TEST_F(FilterTest, TrivialValidatesAll) {
  EXPECT_TRUE(f.valid(0));
  EXPECT_TRUE(f.valid(std::numeric_limits<uint16_t>::max()));
}

TEST_F(FilterTest, ValidateWorks) {
  f.minimum = 3;
  f.maximum = 7;
  EXPECT_FALSE(f.valid(0));
  EXPECT_FALSE(f.valid(2));
  EXPECT_TRUE(f.valid(3));
  EXPECT_TRUE(f.valid(5));
  EXPECT_TRUE(f.valid(7));
  EXPECT_FALSE(f.valid(8));
  EXPECT_FALSE(f.valid(std::numeric_limits<uint16_t>::max()));
}

TEST_F(FilterTest, TrivialPrintsEmpty) {
  EXPECT_TRUE(f.trivial());
  EXPECT_TRUE(f.debug().empty());
}

TEST_F(FilterTest, NonTrivialPrintsSomething) {
  f.rescale_factor = 3;
  f.minimum = 1;
  f.maximum = 7;
  EXPECT_FALSE(f.trivial());
  EXPECT_FALSE(f.debug().empty());
}

TEST_F(FilterTest, FromJson) {
  nlohmann::json j;
  j["min"] = 2;
  j["max"] = 7;
  j["rescale"] = 0.5;
  f = j;
  EXPECT_EQ(f.minimum, 2);
  EXPECT_EQ(f.maximum, 7);
  EXPECT_EQ(f.rescale_factor, 0.5);
}

TEST_F(FilterTest, OneFilter) {
  f.minimum = 3;
  f.maximum = 7;
  f.rescale_factor = 0.5;
  fs.override_filter(5, f);

  EXPECT_EQ(fs.rescale(4, 2), 2);
  EXPECT_TRUE(fs.valid(4, 10));

  EXPECT_EQ(fs.rescale(5, 2), 1);
  EXPECT_FALSE(fs.valid(5, 10));

  EXPECT_EQ(fs.rescale(6, 2), 2);
  EXPECT_TRUE(fs.valid(6, 10));
}

TEST_F(FilterTest, BlanketFilter) {
  f.minimum = 3;
  f.maximum = 7;
  f.rescale_factor = 0.5;
  fs.set_filters(100, f);

  EXPECT_EQ(fs.rescale(1, 2), 1);
  EXPECT_FALSE(fs.valid(1, 10));

  EXPECT_EQ(fs.rescale(5, 2), 1);
  EXPECT_FALSE(fs.valid(5, 10));

  EXPECT_EQ(fs.rescale(70, 2), 1);
  EXPECT_FALSE(fs.valid(70, 10));
}

TEST_F(FilterTest, SetFromJson) {
  nlohmann::json j;

  nlohmann::json j1;
  j1["count"] = 10;
  j1["min"] = 3;
  j1["max"] = 7;
  j1["rescale"] = 0.5;
  j["blanket"] = j1;

  nlohmann::json j2;
  j2["idx"] = 5;
  j2["min"] = 3;
  j2["max"] = 7;
  j2["rescale"] = 0.5;
  j["exceptions"].push_back(j2);

  fs = j;

  // \todo test correct parsing
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
