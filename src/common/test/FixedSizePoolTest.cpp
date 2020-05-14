// Copyright (C) 2020-2020 European Spallation Source, ERIC. See LICENSE file

#include <common/FixedSizePool.h>
#include <test/TestBase.h>

class FixedSizePoolTest : public TestBase {
public:
};

TEST_F(FixedSizePoolTest, Small_Empty) {
  FixedSizePool<FixedSizePoolParams<8, 1>> pool;

  ASSERT_TRUE(pool.NumSlotsUsed == 0);
}

TEST_F(FixedSizePoolTest, Small_1) {
  FixedSizePool<FixedSizePoolParams<8, 1>> pool;

  void *mem = pool.AllocateSlot();

  ASSERT_TRUE(pool.NumSlotsUsed == 1);
  ASSERT_TRUE(mem >= (void *)pool.PoolBytes);
  ASSERT_TRUE(mem < (void *)(pool.PoolBytes + sizeof(pool.PoolBytes)));

  pool.DeallocateSlot(mem);
}

TEST_F(FixedSizePoolTest, Small_NoSpace) {
  FixedSizePool<FixedSizePoolParams<8, 1>> pool;

  void *mem = pool.AllocateSlot();

  void *noSpace = pool.AllocateSlot();
  ASSERT_EQ(noSpace, nullptr);

  pool.DeallocateSlot(mem);
}

TEST_F(FixedSizePoolTest, Small_1_NewlyAllocatedPattern) {
  FixedSizePool<FixedSizePoolParams<8, 1>> pool;
  unsigned char *mem = (unsigned char *)pool.AllocateSlot();
  for (size_t i = 0; i < pool.SlotBytes; ++i) {
    ASSERT_TRUE(mem[i] == pool.MemAllocatedPattern);
  }
  pool.DeallocateSlot(mem);
}

TEST_F(FixedSizePoolTest, Small_1_Fail_DoubleFree) {
  using FixedSizePool_t = FixedSizePool<FixedSizePoolParams<8, 2>>;
  ASSERT_DEATH(
      {
        FixedSizePool_t pool;
        void *mem = pool.AllocateSlot();
        pool.AllocateSlot();
        pool.DeallocateSlot(mem);
        pool.DeallocateSlot(mem);
      },
      "Excepting dealloc slot to be in use"
      /*"Free slots must be unique. Could mean double delete"*/);
}

TEST_F(FixedSizePoolTest, Small_1_Fail_DeallocateWrong) {
  using FixedSizePool_t = FixedSizePool<FixedSizePoolParams<8, 1>>;
  ASSERT_DEATH(
      {
        FixedSizePool_t pool;
        void *mem = pool.AllocateSlot();
        pool.DeallocateSlot(mem);
        pool.DeallocateSlot(mem);
      },
      "Pool is already empty");
}

TEST_F(FixedSizePoolTest, Small_1_Fail_NotEmpty) {
  using FixedSizePool_t = FixedSizePool<FixedSizePoolParams<8, 1>>;
  ASSERT_DEATH(
      {
        FixedSizePool_t pool;
        pool.AllocateSlot();
      },
      "All slots in pool must be empty");
}

TEST_F(FixedSizePoolTest, Small_1_Fail_DirtyRefPattern) {
  using FixedSizePool_t = FixedSizePool<FixedSizePoolParams<8, 1>>;
  ASSERT_DEATH(
      {
        FixedSizePool_t pool;
        void *mem = pool.AllocateSlot();
        pool.DeallocateSlot(mem);
        *(uint32_t *)mem = 0; // dirty the kMemDeleted zone
      },
      "Deleted memory must have reference pattern");
}

TEST_F(FixedSizePoolTest, Large_1) {
  FixedSizePool<FixedSizePoolParams<8, 1000>> pool;

  void *mem = pool.AllocateSlot();

  ASSERT_TRUE(pool.NumSlotsUsed == 1);
  ASSERT_TRUE(mem >= (void *)pool.PoolBytes);
  ASSERT_TRUE(mem < (void *)(pool.PoolBytes + sizeof(pool.PoolBytes)));

  pool.DeallocateSlot(mem);
}

TEST_F(FixedSizePoolTest, Large_All) {
  FixedSizePool<FixedSizePoolParams<8, 1000>> pool;

  void *allocs[1000];
  for (uint32_t i = 0; i < 1000; ++i) {
    allocs[i] = pool.AllocateSlot();

    ASSERT_TRUE(pool.NumSlotsUsed == i + 1);
    ASSERT_TRUE(allocs[i] >= (void *)pool.PoolBytes);
    ASSERT_TRUE(allocs[i] < (void *)(pool.PoolBytes + sizeof(pool.PoolBytes)));
  }

  for (int i = 0; i < 1000; ++i) {
    pool.DeallocateSlot(allocs[i]);
  }
}

TEST_F(FixedSizePoolTest, Large_1mb) {
  // tests the validation can run in proper time on 1 mb pool
  auto pool = new FixedSizePool<FixedSizePoolParams<1, 1ull * 1024 * 1024>>();
  delete pool;
}

TEST_F(FixedSizePoolTest, Align_AtSame) {
  FixedSizePool<FixedSizePoolParams<8, 2, 8>> pool;

  void *mem = pool.AllocateSlot();
  ASSERT_EQ(size_t(mem) % 8, 0);

  void *mem2 = pool.AllocateSlot();
  ASSERT_EQ(size_t(mem2) % 8, 0);

  ASSERT_TRUE(size_t(mem2) - size_t(mem) >= 8);

  pool.DeallocateSlot(mem);
  pool.DeallocateSlot(mem2);
}

TEST_F(FixedSizePoolTest, Align_At64) {
  FixedSizePool<FixedSizePoolParams<8, 2, 64>> pool;

  void *mem = pool.AllocateSlot();
  ASSERT_EQ(size_t(mem) % 64, 0);

  void *mem2 = pool.AllocateSlot();
  ASSERT_EQ(size_t(mem2) % 64, 0);

  ASSERT_TRUE(size_t(mem2) - size_t(mem) >= 8);

  pool.DeallocateSlot(mem);
  pool.DeallocateSlot(mem2);
}

TEST_F(FixedSizePoolTest, Align_At2) {
  FixedSizePool<FixedSizePoolParams<64, 2, 2>> pool;

  void *mem = pool.AllocateSlot();
  ASSERT_EQ(size_t(mem) % 2, 0);

  void *mem2 = pool.AllocateSlot();
  ASSERT_EQ(size_t(mem2) % 2, 0);

  ASSERT_TRUE(size_t(mem2) - size_t(mem) >= 64);

  pool.DeallocateSlot(mem);
  pool.DeallocateSlot(mem2);
}

TEST_F(FixedSizePoolTest, Contains) {
  FixedSizePool<FixedSizePoolParams<8, 2>> pool;

  void *mem = pool.AllocateSlot();
  ASSERT_TRUE(pool.Contains(mem));

  int dummy;
  ASSERT_TRUE(!pool.Contains((void *)&dummy));

  ASSERT_TRUE(!pool.Contains(NULL));

  int *ip = new int;
  ASSERT_TRUE(!pool.Contains(ip));
  delete ip;

  void *mem2 = pool.AllocateSlot();
  ASSERT_TRUE(pool.Contains(mem2));

  pool.DeallocateSlot(mem);
  pool.DeallocateSlot(mem2);
}

TEST_F(FixedSizePoolTest, NextPowerOfTwo) {
  ASSERT_EQ(BitMath::NextPowerOfTwo(0ull), 0ull); // edge case
  ASSERT_EQ(BitMath::NextPowerOfTwo(1ull), 1ull);
  ASSERT_EQ(BitMath::NextPowerOfTwo(3ull), 4ull);
  ASSERT_EQ(BitMath::NextPowerOfTwo(7ull), 8ull);
  ASSERT_EQ(BitMath::NextPowerOfTwo(31ull), 32ull);
  ASSERT_EQ(BitMath::NextPowerOfTwo((1ull << 31) + 1), (1ull << 32));
  ASSERT_EQ(BitMath::NextPowerOfTwo((1ull << 32) + 1), (1ull << 33));
  ASSERT_EQ(BitMath::NextPowerOfTwo((1ull << 47) + 1), (1ull << 48));
  ASSERT_EQ(BitMath::NextPowerOfTwo((1ull << 62) + 1), (1ull << 63));
}

TEST_F(FixedSizePoolTest, Stats) {
  FixedSizePool<FixedSizePoolParams<8, 3>> pool;
  ASSERT_EQ(pool.Stats.AllocCount, 0);
  ASSERT_EQ(pool.Stats.AllocBytes, 0);
  ASSERT_EQ(pool.Stats.DeallocCount, 0);
  ASSERT_EQ(pool.Stats.DeallocBytes, 0);

  void *mem = pool.AllocateSlot(1);
  ASSERT_EQ(pool.Stats.AllocCount, 1);
  ASSERT_EQ(pool.Stats.AllocBytes, 1);
  ASSERT_EQ(pool.Stats.DeallocCount, 0);
  ASSERT_EQ(pool.Stats.DeallocBytes, 0);

  void *mem2 = pool.AllocateSlot(2);
  ASSERT_EQ(pool.Stats.AllocCount, 2);
  ASSERT_EQ(pool.Stats.AllocBytes, 3);
  ASSERT_EQ(pool.Stats.DeallocCount, 0);
  ASSERT_EQ(pool.Stats.DeallocBytes, 0);

  pool.DeallocateSlot(mem);
  ASSERT_EQ(pool.Stats.AllocCount, 2);
  ASSERT_EQ(pool.Stats.AllocBytes, 3);
  ASSERT_EQ(pool.Stats.DeallocCount, 1);
  ASSERT_EQ(pool.Stats.DeallocBytes, 1);

  pool.DeallocateSlot(mem2);
  ASSERT_EQ(pool.Stats.AllocCount, 2);
  ASSERT_EQ(pool.Stats.AllocBytes, 3);
  ASSERT_EQ(pool.Stats.DeallocCount, 2);
  ASSERT_EQ(pool.Stats.DeallocBytes, 3);
}

// inline bool bitset64_get (uint64_t* array, uint64_t arrayBitCount, uint64_t
// index)
//{
//  assert(index < arrayBitCount);
//  return (array[index >> 6] & (1ULL << (index & 63))) != 0;
//}
//
// inline void bitset64_raise (uint64_t* array, uint64_t arrayBitCount, uint64_t
// index)
//{
//  assert(index < arrayBitCount);
//  array[index >> 6] |= (1ULL << (index & 63));
//}
//
// inline void bitset64_clear (uint64_t* array, uint64_t arrayBitCount, uint64_t
// index)
//{
//  assert(index < arrayBitCount);
//  array[index >> 6] &= ~(1ULL << (index & 63));
//}
//
// TEST_F(FixedSizePoolTest, ArrayBit64) {
//  static const uint64_t bitCount = 1;
//  uint64_t bitArray[bitCount / 64 + 1];
//  memset(bitArray, 0, sizeof(bitArray));
//  uint64_t setBitIndex = 0;
//  bitset64_raise(bitArray, bitCount, setBitIndex);
//  for (uint64_t i = 0; i < bitCount; ++i) {
//    ASSERT_EQ(bitset64_get(bitArray, bitCount, setBitIndex), i ==
//    setBitIndex);
//  }
//}