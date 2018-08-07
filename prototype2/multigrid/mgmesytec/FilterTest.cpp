/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/mgmesytec/Filter.h>
#include <test/TestBase.h>

using namespace Multigrid;

class FilterTest : public TestBase {
protected:
  Filter f;
  virtual void SetUp() {
  }
  virtual void TearDown() {
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
  EXPECT_FALSE(f.trivial());
  EXPECT_FALSE(f.debug().empty());
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
