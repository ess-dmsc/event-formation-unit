#include <test/TestBase.h>
#include <common/PoolAllocator.h>

class PoolAllocatorTest : public TestBase {
public:
};

TEST_F(PoolAllocatorTest, Small_Empty) {
  PoolAllocator<int, sizeof(int) * 1, 1> alloc;
}

TEST_F(PoolAllocatorTest, Small_1) {
  std::vector<int, PoolAllocator<int, sizeof(int) * 1, 1>> v;
  v.push_back(1);
  ASSERT_TRUE(v[0] == 1);
}

TEST_F(PoolAllocatorTest, Small_Overflow) {
  using vecType = std::vector<int, PoolAllocator<int, sizeof(int) * 1, 1>>;
  ASSERT_DEATH(
      {
        vecType v;
        v.push_back(1);
        v.push_back(1);
      },
      ""); // this gives a segfault
}

TEST_F(PoolAllocatorTest, Small_2) {
  std::vector<int, PoolAllocator<int, sizeof(int) * 2, 2>> v;
  v.reserve(2);
  v.push_back(1);
  v.push_back(2);
  ASSERT_TRUE(v[0] == 1);
  ASSERT_TRUE(v[1] == 2);
}