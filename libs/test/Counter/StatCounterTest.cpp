/** Copyright (C) 2016 European Spallation Source */

#include <libs/include/StatCounter.h>
#include <gtest/gtest.h>

class StatCounterTest : public ::testing::Test {

protected:
  StatCounter<int> icount;
  StatCounter<long> lcount;
  StatCounter<float> fcount;
  StatCounter<double> dcount;
};

TEST_F(StatCounterTest, NewIsZero) {
  ASSERT_EQ(icount.count(), 0);
  ASSERT_EQ(0, icount.sum());
  ASSERT_EQ(0, icount.min());
  ASSERT_EQ(0, icount.max());
  ASSERT_EQ(0, icount.avg());

  ASSERT_EQ(lcount.count(), 0);
  ASSERT_EQ(0, lcount.sum());
  ASSERT_EQ(0, lcount.min());
  ASSERT_EQ(0, lcount.max());
  ASSERT_EQ(0, lcount.avg());

  ASSERT_EQ(fcount.count(), 0);
  ASSERT_EQ(0.0, fcount.sum());
  ASSERT_EQ(0.0, fcount.min());
  ASSERT_EQ(0.0, fcount.max());
  ASSERT_EQ(0.0, fcount.avg());

  ASSERT_EQ(dcount.count(), 0);
  ASSERT_EQ(0.0, dcount.sum());
  ASSERT_EQ(0.0, dcount.min());
  ASSERT_EQ(0.0, dcount.max());
  ASSERT_EQ(0.0, dcount.avg());
}

TEST_F(StatCounterTest, TestMinMax) {
  for (int i = 5; i <= 999999; i++) {
    icount.add(i);
    lcount.add((long long)i);
    fcount.add((float)(1.0 * i));
    dcount.add((double)(1.0 * i));
  }
  ASSERT_EQ(999995, icount.count());
  ASSERT_EQ(icount.max(), 999999);
  ASSERT_EQ(icount.min(), 5);

  ASSERT_EQ(999995, lcount.count());
  ASSERT_EQ(lcount.max(), 999999);
  ASSERT_EQ(lcount.min(), 5);

  ASSERT_EQ(999995, fcount.count());
  ASSERT_EQ(fcount.max(), 999999.0);
  ASSERT_EQ(fcount.min(), 5.0);

  ASSERT_EQ(999995, dcount.count());
  ASSERT_EQ(dcount.max(), 999999.0);
  ASSERT_EQ(dcount.min(), 5.0);
}

TEST_F(StatCounterTest, TestAverage) {
  for (int i = 1; i <= 100; i++) {
    icount.add(10 * i);
    lcount.add(10 * i);
    fcount.add(10.0 * i);
    dcount.add(10.0 * i);
  }
  ASSERT_EQ(icount.avg(), 505);
  ASSERT_EQ(lcount.avg(), 505);
  ASSERT_EQ(fcount.avg(), 505);
  ASSERT_EQ(dcount.avg(), 505);
}
