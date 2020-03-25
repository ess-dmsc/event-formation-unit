#include <test/TestBase.h>
#include <common/FixedSizePool.h>

class FixedSizePoolTest : public TestBase {
public:
};

TEST_F(FixedSizePoolTest, Small_Empty) {
  FixedSizePool<8, 1> pool;

  ASSERT_TRUE(pool.m_NumSlotsUsed == 0);
}

TEST_F(FixedSizePoolTest, Small_1) {
  FixedSizePool<8, 1> pool;

  void *mem = pool.AllocateSlot();

  ASSERT_TRUE(pool.m_NumSlotsUsed == 1);
  ASSERT_TRUE(mem >= (void *)pool.m_PoolBytes);
  ASSERT_TRUE(mem < (void *)(pool.m_PoolBytes + sizeof(pool.m_PoolBytes)));

  pool.DeallocateSlot(mem);
}

TEST_F(FixedSizePoolTest, Small_1_NewlyAllocatedPattern) {
  FixedSizePool<8, 1> pool;
  unsigned char *mem = (unsigned char *)pool.AllocateSlot();
  for (size_t i = 0; i < pool.kSlotBytes; ++i) {
    ASSERT_TRUE(mem[i] == pool.kMemAllocatedPat);
  }
  pool.DeallocateSlot(mem);
}

TEST_F(FixedSizePoolTest, Small_1_Fail_DoubleFree) {
  using FixedSizePool_t = FixedSizePool<8, 2>;
  ASSERT_DEATH(
      {
        FixedSizePool_t pool;
        void *mem = pool.AllocateSlot();
        pool.AllocateSlot();
        pool.DeallocateSlot(mem);
        pool.DeallocateSlot(mem);
      },
      "Free slots must be unique. Could mean double delete");
}

TEST_F(FixedSizePoolTest, Small_1_Fail_DeallocateWrong) {
  using FixedSizePool_t = FixedSizePool<8, 1>;
  ASSERT_DEATH(
      {
        FixedSizePool_t pool;
        void *mem = pool.AllocateSlot();
        pool.DeallocateSlot(mem);
        pool.DeallocateSlot(mem);
      },
      "Pool must have content");
}

TEST_F(FixedSizePoolTest, Small_1_Fail_NotEmpty) {
  using FixedSizePool_t = FixedSizePool<8, 1>;
  ASSERT_DEATH(
      {
        FixedSizePool_t pool;
        pool.AllocateSlot();
      },
      "All slots in pool must be empty");
}

TEST_F(FixedSizePoolTest, Small_1_Fail_DirtyRefPattern) {
  using FixedSizePool_t = FixedSizePool<8, 1>;
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
  FixedSizePool<8, 1000> pool;

  void *mem = pool.AllocateSlot();

  ASSERT_TRUE(pool.m_NumSlotsUsed == 1);
  ASSERT_TRUE(mem >= (void *)pool.m_PoolBytes);
  ASSERT_TRUE(mem < (void *)(pool.m_PoolBytes + sizeof(pool.m_PoolBytes)));

  pool.DeallocateSlot(mem);
}

TEST_F(FixedSizePoolTest, Large_All) {
  FixedSizePool<8, 1000> pool;

  void *allocs[1000];
  for (uint32_t i = 0; i < 1000; ++i) {
    allocs[i] = pool.AllocateSlot();

    ASSERT_TRUE(pool.m_NumSlotsUsed == i + 1);
    ASSERT_TRUE(allocs[i] >= (void *)pool.m_PoolBytes);
    ASSERT_TRUE(allocs[i] <
                (void *)(pool.m_PoolBytes + sizeof(pool.m_PoolBytes)));
  }

  for (int i = 0; i < 1000; ++i) {
    pool.DeallocateSlot(allocs[i]);
  }
}

TEST_F(FixedSizePoolTest, Align_AtSame) {
  FixedSizePool<8, 2, 8> pool;

  void *mem = pool.AllocateSlot();
  ASSERT_EQ(size_t(mem) % 8, 0);

  void *mem2 = pool.AllocateSlot();
  ASSERT_EQ(size_t(mem2) % 8, 0);

  ASSERT_TRUE(size_t(mem2) - size_t(mem) >= 8);

  pool.DeallocateSlot(mem);
  pool.DeallocateSlot(mem2);
}

TEST_F(FixedSizePoolTest, Align_At64) {
  FixedSizePool<8, 2, 64> pool;

  void *mem = pool.AllocateSlot();
  ASSERT_EQ(size_t(mem) % 64, 0);

  void *mem2 = pool.AllocateSlot();
  ASSERT_EQ(size_t(mem2) % 64, 0);

  ASSERT_TRUE(size_t(mem2) - size_t(mem) >= 8);

  pool.DeallocateSlot(mem);
  pool.DeallocateSlot(mem2);
}

TEST_F(FixedSizePoolTest, Align_At2) {
  FixedSizePool<64, 2, 2> pool;

  void *mem = pool.AllocateSlot();
  ASSERT_EQ(size_t(mem) % 2, 0);

  void *mem2 = pool.AllocateSlot();
  ASSERT_EQ(size_t(mem2) % 2, 0);

  ASSERT_TRUE(size_t(mem2) - size_t(mem) >= 64);

  pool.DeallocateSlot(mem);
  pool.DeallocateSlot(mem2);
}