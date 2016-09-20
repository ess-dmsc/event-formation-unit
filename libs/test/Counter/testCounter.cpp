#include <Counter.h>
#include <gtest/gtest.h>

class CounterTest : public ::testing::Test {

protected:
  Counter<int> icount;
  Counter<long long> lcount;
  Counter<float> fcount;
  Counter<double> dcount;
};

TEST_F(CounterTest, NewIsEmpty) {
  ASSERT_TRUE(icount.empty());
  ASSERT_TRUE(lcount.empty());
  ASSERT_TRUE(fcount.empty());
  ASSERT_TRUE(dcount.empty());
}

TEST_F(CounterTest, AddedNotEmpty) {

  icount.add(42);
  ASSERT_FALSE(icount.empty());

  lcount.add(42);
  ASSERT_FALSE(lcount.empty());

  fcount.add(4.2);
  ASSERT_FALSE(fcount.empty());

  dcount.add(4.2);
  ASSERT_FALSE(dcount.empty());
}

TEST_F(CounterTest, TestMinMax) {
  for (int i = 1; i <= 999999; i++) {
    icount.add(i);
    lcount.add((long long)i);
    fcount.add(1.0 * i);
    dcount.add(1.0 * i);
  }
  ASSERT_EQ(icount.max(), 999999);
  ASSERT_EQ(icount.min(), 1);

  ASSERT_EQ(lcount.max(), 999999);
  ASSERT_EQ(lcount.min(), 1);

  EXPECT_NEAR(fcount.max(), 999999.0, 0.00001);
  EXPECT_NEAR(fcount.min(), 1.0, 0.00001);

  EXPECT_NEAR(dcount.max(), 999999.0, 0.00001);
  EXPECT_NEAR(dcount.min(), 1.0, 0.00001);
}

TEST_F(CounterTest, TestAverage) {
  ASSERT_TRUE(false); /** not implemented yet */
}
