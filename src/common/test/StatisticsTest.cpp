// Copyright (C) 2016 - 2025 European Spallation Source ERIC

#include <common/Statistics.h>
#include <common/testutils/TestBase.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <iostream>

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
}

TEST_F(NewStatsTest, CreateStatPrefix) {
  Statistics stats("dmsc.efu", "0");
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
  std::string DotPrefix = "some_prefix.";
  std::string Region = "0.";
  std::string SomeStat = "stat1";
  int64_t ctr1 = 765;
  Statistics stats(DotPrefix, Region);
  stats.create(SomeStat, ctr1);
  ASSERT_EQ(1U, stats.size());
  ASSERT_EQ(DotPrefix + Region + SomeStat, stats.getFullName(1));
}

TEST_F(NewStatsTest, CreateStatPrefixWitoutDot) {
  int64_t ctr1 = 765;
  std::string DotPrefix = "some_prefix";
  std::string Region = "0";
  std::string SomeStat = "stat1";
  Statistics stats(DotPrefix, Region);
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

TEST_F(NewStatsTest, TestThreadSafetyWithMultipleThreads) {

  // stats object to test
  Statistics stats;

  // start synchronization
  std::atomic<bool> start_signal{false};

  // maximum of create operations
  const int MAX_OPERATIONS = 200;
  std::vector<int64_t> counters(MAX_OPERATIONS, 2); // Exactly match the operation count
  
  // Helper function to synchronize thread start times
  // This ensures all threads begin their actual work simultaneously,
  // maximizing the chance of detecting race conditions in the locking mechanism
  auto wait_for_start = [&]() {
    while (!start_signal.load()) std::this_thread::yield();
  };

  // Thread pool for concurrent operations
  std::vector<std::thread> threads;
  
  // Writer thread - Continuously creates new statistics and modifies existing ones
  // This thread performs write operations that require exclusive locks:
  // - create(): adds new StatTuple entries to the ThreadSafeVector
  // - setStatName(): modifies existing StatTuple names 
  // - setStatPrefix(): modifies existing StatTuple prefixes
  threads.emplace_back([&]() {
    wait_for_start();
    
    for (int op = 0; op < MAX_OPERATIONS; op++) {
      // Create new stats (write operation - exclusive lock)
      stats.create("stat" + std::to_string(op), 
                   counters[op], 
                   "prefix" + std::to_string(op % 3) + ".");
      
      // Modify existing stats (write operations - exclusive lock)
      size_t count = stats.size();
      if (count > 0) {
        size_t idx = (op % count) + 1;
        stats.setStatName(idx, "mod_" + std::to_string(op));
        stats.setStatPrefix(idx, "chg" + std::to_string(op % 2) + ".");
      }
      
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  });
  
  // Reader thread 1 - Continuously reads all statistics using index-based access
  // This thread performs read operations that should use shared locks:
  // - size(): gets count of statistics
  // - getStatName(): reads StatTuple name by index
  // - getFullName(): constructs full name from prefix + name
  // - getValue(): reads StatTuple value by index
  threads.emplace_back([&]() {
    wait_for_start();
    
    for (int reads = 0; reads < 300; reads++) {
      size_t count = stats.size(); // Read operation - shared lock
      for (size_t i = 1; i <= count; i++) {
        // Multiple read operations - each should acquire shared lock
        std::string name = stats.getStatName(i);
        std::string fullName = stats.getFullName(i);
        int64_t value = stats.getValue(i);
        
        // Verify data consistency during concurrent modifications
        EXPECT_FALSE(name.empty());
        EXPECT_EQ(value, 0); // create() sets to 0
        EXPECT_TRUE(fullName.find(name) != std::string::npos);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
  });
  
  // Reader thread 2 - Continuously reads statistics using name-based lookup
  // This thread tests the search functionality under concurrent modifications:
  // - getValueByName(): searches through all StatTuple entries to find matches
  // This is more complex as it involves iterating through the entire vector
  threads.emplace_back([&]() {
    wait_for_start();
    
    for (int reads = 0; reads < 300; reads++) {
      for (int i = 0; i < 50; i++) { // Search for stats being created by writer
        // Search operations - should use shared locks for safe iteration
        int64_t val1 = stats.getValueByName("stat" + std::to_string(i), "prefix0.");
        int64_t val2 = stats.getValueByName("stat" + std::to_string(i), "prefix1.");
        int64_t val3 = stats.getValueByName("stat" + std::to_string(i), "prefix2.");
        
        // Verify search results: 0 if found, -1 if not found
        EXPECT_TRUE(val1 == 0 || val1 == -1); // found or not found
        EXPECT_TRUE(val2 == 0 || val2 == -1);
        EXPECT_TRUE(val3 == 0 || val3 == -1);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  });
  
  // Start all threads simultaneously to maximize lock contention
  start_signal = true;
  
  // Wait for all threads to complete their work naturally
  for (auto& t : threads) t.join();
  
  size_t final_size = stats.size();
  EXPECT_EQ(final_size, MAX_OPERATIONS); // Should have created exactly MAX_OPERATIONS stats
  
  for (size_t i = 1; i <= final_size; i++) {
    EXPECT_FALSE(stats.getStatName(i).empty());
    EXPECT_EQ(stats.getValue(i), 0);
    EXPECT_FALSE(stats.getStatPrefix(i).empty());
  }
  
  std::cout << "Thread safety test completed with " << final_size << " final stats" << std::endl;
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
