/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include "TestBase.h"
#include <common/NewStats.h>

class NewStatsTest : public TestBase {};

TEST_F(NewStatsTest, Constructor) {
  NewStats stats;
  ASSERT_EQ(0, stats.size());

  ASSERT_EQ("", stats.name(0));
}

TEST_F(NewStatsTest, CreateStat) {
  NewStats stats;
  int64_t ctr1 = 765;
  int64_t ctr2 = 432;

  ASSERT_EQ(0, stats.size());

  stats.create(std::string("stat1"), &ctr1);
  ASSERT_EQ(1, stats.size());
  ASSERT_EQ("stat1", stats.name(1));

  stats.create(std::string("stat2"), &ctr2);
  ASSERT_EQ(2, stats.size());
  ASSERT_EQ("stat2", stats.name(2));
}

TEST_F(NewStatsTest, DuplicateStat) {
  NewStats stats;
  int64_t ctr1;
  int64_t ctr2;

  ASSERT_EQ(0, stats.size());

  int res = stats.create(std::string("stat1"), &ctr1);
  ASSERT_EQ(0, res);
  ASSERT_EQ(1, stats.size());

  res = stats.create(std::string("stat1"), &ctr2);
  ASSERT_EQ(-1, res);
  ASSERT_EQ(1, stats.size());

  res = stats.create(std::string("stat2"), &ctr1);
  ASSERT_EQ(-1, res);
  ASSERT_EQ(1, stats.size());
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
