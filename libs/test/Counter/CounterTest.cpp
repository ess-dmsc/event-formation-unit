/** Copyright (C) 2016 European Spallation Source */

#include <libs/include/Counter.h>
#include <gtest/gtest.h>

class CounterTest : public ::testing::Test {

protected:
  Counter count;
};

TEST_F(CounterTest, NewIsEmpty) {
  ASSERT_EQ(0, count.count());
}

TEST_F(CounterTest, Add) {
  for (int i = 0; i < 98765; i++) {
    count.add();
    ASSERT_EQ(i + 1, count.count());
  }
}

TEST_F(CounterTest, Clear) {
  for (int i = 0; i < 98765; i++) {
    count.add();
    ASSERT_EQ(i + 1, count.count());
  }
  count.clear();
  ASSERT_EQ(0, count.count());
}
