/** Copyright (C) 2016 European Spallation Source */

#include <cstdlib>
#include <functional>
#include <gtest/gtest.h>
#include <iostream>
#include <queue>

template <typename T> void print_queue(T &q) {
  while (!q.empty()) {
    std::cout << q.top() << " ";
    q.pop();
  }
  std::cout << "\n";
}

int myints[] = {1, 5, 2, 8, 7, 3, 11};

TEST(PriorityQueue, MinimumPQ) {

  std::priority_queue<int, std::vector<int>, std::greater<int>> pq(myints,
                                                                   myints + 7);

  ASSERT_EQ(pq.top(), 1);

  if (pq.top() != 1) {
    print_queue(pq);
  }
}

TEST(PriorityQueue, MaximumPQ) {
  std::priority_queue<int> pq(myints, myints + 7);

  ASSERT_EQ(pq.top(), 11);

  if (pq.top() != 11) {
    print_queue(pq);
  }
}
