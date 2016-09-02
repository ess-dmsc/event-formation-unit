#include <cstdlib>
#include <functional>
#include <gtest/gtest.h>
#include <iostream>
#include <queue>

TEST(PriorityQueue, BasicTests) {
  int myints[] = {1, 5, 2, 8, 7, 3, 11};

  std::priority_queue<int> initializedpq(myints, myints + 7);

  ASSERT_EQ(initializedpq.size(), 7)
      << "Priority queue not initialized correctly";

  std::priority_queue<int> emptypq;

  ASSERT_EQ(emptypq.size(), 0)
      << "Uninitialized Priority Queue should be empty";
}
