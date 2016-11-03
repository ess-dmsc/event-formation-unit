/** Copyright (C) 2016 European Spallation Source */

#include <libs/include/StatCounter.h>
#include <gtest/gtest.h>

class StatCounterTest : public ::testing::Test {

protected:
  StatCounter<int> icount;
};

TEST_F(StatCounterTest, NewIsZero) { ASSERT_EQ(icount.counts(), 0); }

TEST_F(CounterTest, TestMinMax) {
  for (int i = 1; i <= 999999; i++) {
    icount.add(i);
  }
  ASSERT_EQ(icount.max(), 999999);
  ASSERT_EQ(icount.min(), 1);
}

TEST_F(CounterTest, TestAverage) {
  for (int i = 1; i <= 100; i++) {
    icount.add(10 * i);
  }
  ASSERT_EQ(icount.avg(), 505);
}
