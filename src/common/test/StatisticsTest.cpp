// Copyright (C) 2016, 2017 European Spallation Source ERIC

#include <common/Statistics.h>
#include <common/StatCounterBase.h>
#include <common/testutils/TestBase.h>

class NewStatsTest : public TestBase {};

TEST_F(NewStatsTest, Constructor) {
  Statistics stats;
  ASSERT_EQ(stats.size(), 0U);
  ASSERT_EQ("", stats.getStatName(0));
}

TEST_F(NewStatsTest, ConstructorDynamic) {
  auto stats = new Statistics();

  ASSERT_EQ(stats->size(), 0U);
  ASSERT_EQ("", stats->getStatName(0));

  delete stats;
  stats = 0;
}

TEST_F(NewStatsTest, CreateStat) {
  Statistics stats;
  int64_t ctr1 = 765;
  int64_t ctr2 = 432;

  ASSERT_EQ(0U, stats.size());

  stats.create(std::string("stat1"), ctr1);
  ASSERT_EQ(1U, stats.size());
  ASSERT_EQ("stat1", stats.getStatName(1));

  stats.create(std::string("stat2"), ctr2);
  ASSERT_EQ(2U, stats.size());
  ASSERT_EQ("stat2", stats.getStatName(2));
}

TEST_F(NewStatsTest, ValueByName) {
  Statistics stats;
  int64_t ctr1 = 765;
  int64_t ctr2 = 432;
  int64_t ctr3 = 999;
  int64_t ctr4 = 111;

  // Create stats with default prefix (empty)
  stats.create(std::string("stat1"), ctr1);
  stats.create(std::string("stat2"), ctr2);

  // Create stats with custom prefixes
  stats.create(std::string("stat1"), ctr3, "prefix1.");
  stats.create(std::string("stat2"), ctr4, "prefix2.");

  // Test retrieval with default prefix (empty)
  ASSERT_EQ(ctr1, stats.getValueByName("stat1"));
  ASSERT_EQ(ctr2, stats.getValueByName("stat2"));

  // Test retrieval with explicit empty prefix
  ASSERT_EQ(ctr1, stats.getValueByName("stat1", ""));
  ASSERT_EQ(ctr2, stats.getValueByName("stat2", ""));

  // Test retrieval with specific prefixes
  ASSERT_EQ(ctr3, stats.getValueByName("stat1", "prefix1."));
  ASSERT_EQ(ctr4, stats.getValueByName("stat2", "prefix2."));

  // Test non-existent name
  ASSERT_EQ(-1, stats.getValueByName("nonexistent"));

  // Test existing name with non-existent prefix
  ASSERT_EQ(-1, stats.getValueByName("stat1", "nonexistent."));

  // Set common prefix and test
  stats.setPrefix("common", "prefix");
  int64_t ctr5 = 555;
  stats.create(std::string("stat3"), ctr5);

  // Test retrieval with common prefix
  ASSERT_EQ(ctr5, stats.getValueByName("stat3", "common.prefix."));
  // Default should now use common prefix
  ASSERT_EQ(ctr5, stats.getValueByName("stat3"));
}

TEST_F(NewStatsTest, CreateStatPrefix) {
  Statistics stats;
  stats.setPrefix("dmsc.efu", "0");
  int64_t ctr1 = 765;
  int64_t ctr2 = 432;

  ASSERT_EQ(0U, stats.size());

  stats.create(std::string("stat1"), ctr1);
  ASSERT_EQ(1U, stats.size());
  ASSERT_EQ("stat1", stats.getStatName(1));
  ASSERT_EQ("dmsc.efu.0.", stats.getStatPrefix(1));
  ASSERT_EQ("dmsc.efu.0.stat1", stats.getFullName(1));

  stats.create(std::string("stat2"), ctr2);
  ASSERT_EQ(2U, stats.size());
  ASSERT_EQ("stat2", stats.getStatName(2));
  ASSERT_EQ("dmsc.efu.0.", stats.getStatPrefix(2));
  ASSERT_EQ("dmsc.efu.0.stat2", stats.getFullName(2));

  ASSERT_EQ("", stats.getStatName(3));
}

TEST_F(NewStatsTest, CreateStatPrefixWithDot) {
  Statistics stats;
  int64_t ctr1 = 765;
  std::string DotPrefix = "some_prefix.";
  std::string Region = "0.";
  std::string SomeStat = "stat1";
  stats.setPrefix(DotPrefix, Region);
  stats.create(SomeStat, ctr1);
  ASSERT_EQ(1U, stats.size());
  ASSERT_EQ(DotPrefix + Region + SomeStat, stats.getFullName(1));
}

TEST_F(NewStatsTest, CreateStatPrefixWitoutDot) {
  Statistics stats;
  int64_t ctr1 = 765;
  std::string DotPrefix = "some_prefix";
  std::string Region = "0";
  std::string SomeStat = "stat1";
  stats.setPrefix(DotPrefix, "0");
  stats.create(SomeStat, ctr1);
  ASSERT_EQ(1U, stats.size());
  ASSERT_EQ(DotPrefix + "." + Region + "." + SomeStat, stats.getFullName(1));
}

TEST_F(NewStatsTest, DuplicateStatPrefix) {
  Statistics stats;
  int64_t ctr1;
  int64_t ctr2;

  ASSERT_EQ(0U, stats.size());

  int res = stats.create(std::string("stat1"), ctr1);
  ASSERT_EQ(0, res);
  ASSERT_EQ(1U, stats.size());

  res = stats.create(std::string("stat1"), ctr2);
  ASSERT_EQ(-1, res);
  ASSERT_EQ(1U, stats.size());

  res = stats.create(std::string("stat2"), ctr1);
  ASSERT_EQ(-1, res);
  ASSERT_EQ(1U, stats.size());
}

TEST_F(NewStatsTest, DuplicateStat) {
  Statistics stats;
  int64_t ctr1;
  int64_t ctr2;

  ASSERT_EQ(0U, stats.size());

  int res = stats.create(std::string("stat1"), ctr1);
  ASSERT_EQ(0, res);
  ASSERT_EQ(1U, stats.size());

  res = stats.create(std::string("stat1"), ctr2);
  ASSERT_EQ(-1, res);
  ASSERT_EQ(1U, stats.size());

  res = stats.create(std::string("stat2"), ctr1);
  ASSERT_EQ(-1, res);
  ASSERT_EQ(1U, stats.size());
}

TEST_F(NewStatsTest, StatValue) {
  Statistics stats;
  int64_t ctr1 = 0;

  ASSERT_EQ(-1, stats.getValue(0));
  ASSERT_EQ(-1, stats.getValue(1));

  int res = stats.create(std::string("stat1"), ctr1);
  ASSERT_EQ(0, res);
  ASSERT_EQ(1U, stats.size());

  ASSERT_EQ(ctr1, stats.getValue(1));

  ctr1 = INT64_MAX;
  ASSERT_EQ(INT64_MAX, stats.getValue(1));

  ctr1++; // cppcheck-suppress unreadVariable // IS read by stats.value()
  ASSERT_EQ(INT64_MIN, stats.getValue(1));
}

TEST_F(NewStatsTest, StatCounterBaseRegistration) {
  Statistics stats;
  
  // Test struct that uses StatCounterBase for automatic registration
  struct TestCounters : public StatCounterBase {
    int64_t CounterA{0};
    int64_t CounterB{42};

    TestCounters(Statistics &Stats)
        : StatCounterBase(Stats, {{"a", CounterA}, {"b", CounterB}}, "dummy") {}
  };
  
  TestCounters counters(stats);

  // Verify counters are registered with correct initial values
  ASSERT_EQ(stats.size(), 2U);
  ASSERT_EQ(stats.getValueByName("dummy.a"), 0);
  ASSERT_EQ(stats.getValueByName("dummy.b"), 42);

  // Update counters and verify they reflect in Statistics
  counters.CounterA = 5;
  counters.CounterB = 99;
  
  ASSERT_EQ(stats.getValueByName("dummy.a"), 5);
  ASSERT_EQ(stats.getValueByName("dummy.b"), 99);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
