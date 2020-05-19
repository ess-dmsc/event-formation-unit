// Copyright (C) 2020-2020 European Spallation Source, ERIC. See LICENSE file

#include <common/PoolAllocator.h>
#include <test/TestBase.h>

class PoolAllocatorTest : public TestBase {
public:
};

TEST_F(PoolAllocatorTest, Small_Empty) {
  using AllocConfig = PoolAllocatorConfig<int, sizeof(int) * 1, 1, true>;
  AllocConfig::PoolType pool;
  PoolAllocator<AllocConfig> alloc(pool);
  ASSERT_STREQ(pool.ValidateEmptyStateAndReturnError(), nullptr);
}

TEST_F(PoolAllocatorTest, Small_1) {
  using AllocConfig = PoolAllocatorConfig<int, sizeof(int) * 1, 1, true>;
  AllocConfig::PoolType pool;
  PoolAllocator<AllocConfig> alloc(pool);
  {
    std::vector<int, PoolAllocator<AllocConfig>> v(alloc);
    v.push_back(1);
    ASSERT_TRUE(v[0] == 1);
  }
  ASSERT_STREQ(pool.ValidateEmptyStateAndReturnError(), nullptr);
}

TEST_F(PoolAllocatorTest, Small_OverflowSlotSize) {
  using AllocConfig = PoolAllocatorConfig<int, sizeof(int) * 1, 1, true>;
  AllocConfig::PoolType pool;
  PoolAllocator<AllocConfig> alloc(pool);
  {
    using vecType = std::vector<int, PoolAllocator<AllocConfig>>;
    vecType v(alloc);

    v.push_back(1);
    ASSERT_EQ(pool.Contains(v.data()), true);
    ASSERT_EQ(pool.Stats.MallocFallbackCount, 0);

    v.push_back(1);
    ASSERT_EQ(pool.Contains(v.data()), false);
    ASSERT_EQ(pool.Stats.MallocFallbackCount, 1);
  }
  ASSERT_STREQ(pool.ValidateEmptyStateAndReturnError(), nullptr);
}

TEST_F(PoolAllocatorTest, Small_OverflowPoolCapacity) {
  using AllocConfig = PoolAllocatorConfig<int, sizeof(int) * 1, 1, true>;
  AllocConfig::PoolType pool;
  PoolAllocator<AllocConfig> alloc(pool);
  {
    using vecType = std::vector<int, PoolAllocator<AllocConfig>>;

    vecType v1(alloc);
    v1.push_back(1);
    ASSERT_EQ(pool.Contains(v1.data()), true);
    ASSERT_EQ(pool.Stats.MallocFallbackCount, 0);

    vecType v2(alloc);
    v2.push_back(1);
    ASSERT_EQ(pool.Contains(v2.data()), false);
    ASSERT_EQ(pool.Stats.MallocFallbackCount, 1);

    vecType v3(alloc);
    v3.push_back(1);
    ASSERT_EQ(pool.Contains(v3.data()), false);
    ASSERT_EQ(pool.Stats.MallocFallbackCount, 2);
  }
  ASSERT_STREQ(pool.ValidateEmptyStateAndReturnError(), nullptr);
}

TEST_F(PoolAllocatorTest, Small_2) {
  using AllocConfig = PoolAllocatorConfig<int, sizeof(int) * 2, 2, true>;
  AllocConfig::PoolType pool;
  PoolAllocator<AllocConfig> alloc(pool);
  {
    std::vector<int, decltype(alloc)> v(alloc);
    v.reserve(2);
    v.push_back(1);
    v.push_back(2);
    ASSERT_TRUE(v[0] == 1);
    ASSERT_TRUE(v[1] == 2);
  }
  ASSERT_STREQ(pool.ValidateEmptyStateAndReturnError(), nullptr);
}

TEST_F(PoolAllocatorTest, Share_2) {
  using AllocConfig = PoolAllocatorConfig<int, (sizeof(int) * 2) * 2, 2, true>;
  AllocConfig::PoolType pool;
  PoolAllocator<AllocConfig> alloc(pool);
  {
    ASSERT_EQ(alloc.Pool.NumSlotsUsed, 0);

    std::vector<int, decltype(alloc)> v1(alloc);
    v1.reserve(1);
    v1.push_back(1);

    ASSERT_EQ(alloc.Pool.NumSlotsUsed, 1);

    std::vector<int, decltype(alloc)> v2(alloc);
    v2.reserve(1);
    v2.push_back(2);

    ASSERT_EQ(alloc.Pool.NumSlotsUsed, 2);

    ASSERT_EQ(v1[0], 1);
    ASSERT_EQ(v2[0], 2);
  }
  ASSERT_EQ(pool.NumSlotsUsed, 0);
  ASSERT_STREQ(pool.ValidateEmptyStateAndReturnError(), nullptr);
}
