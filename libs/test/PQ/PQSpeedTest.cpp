/** Copyright (C) 2016 European Spallation Source */

#include <cstdlib>
#include <functional>
#include <gtest/gtest.h>
#include <iostream>
#include <libs/include/Timer.h>
#include <queue>

class PriorityQueueTest : public ::testing::Test {
protected:
  virtual void SetUp() { std::srand(4242); }

  virtual void TearDown() {
    ASSERT_EQ(x.size(), y.size());
    for (int i = 0; i < (int)x.size(); i++)
      std::cout << x[i] << " , " << y[i] << "\n";
  }

  std::vector<int> pqsizes{250000,  500000,  750000,  1000000, 1500000,
                           2000000, 3000000, 4000000, 5000000};

  std::priority_queue<int, std::vector<int>, std::greater<int>> pq;
  std::vector<int> x, y;
  int random_variable;
};

TEST_F(PriorityQueueTest, DoublingTestRandom) {
  for (auto const &count : pqsizes) {
    std::cout << "inserting " << count << " random ints -";

    Timer tm;
    for (int i = 0; i < count; i++) {
      random_variable = std::rand();
      pq.push(random_variable);
    }

    for (int i = 0; i < count; i++) {
      pq.pop();
    }
    x.push_back(count);
    y.push_back(tm.timeus());
    std::cout << " time (us): " << tm.timeus() << "\n";
  }
}

TEST_F(PriorityQueueTest, RandomSpeed) {
  for (auto const &count : pqsizes) {
    std::cout << "generating " << count << " random ints -";

    Timer tm;
    for (int i = 0; i < count; i++) {
      int random_variable[[gnu::unused]] = std::rand();
    }
    x.push_back(count);
    y.push_back(tm.timeus());
    std::cout << " time (us): " << tm.timeus() << "\n";
  }
}

TEST_F(PriorityQueueTest, AlreadySortedIncreasing) {
  for (auto const &count : pqsizes) {
    std::cout << "generating " << count << " sorted ints (increasing) -";

    Timer tm;
    for (int i = 0; i < count; i++) {
      pq.push(i);
    }

    for (int i = 0; i < count; i++) {
      pq.pop();
    }

    x.push_back(count);
    y.push_back(tm.timeus());
    std::cout << " time (us): " << tm.timeus() << "\n";
  }
}

TEST_F(PriorityQueueTest, AlreadySortedDecreasing) {
  for (auto const &count : pqsizes) {
    std::cout << "generating " << count << " sorted ints (decreasing)-";

    Timer tm;
    for (int i = 0; i < count; i++) {
      pq.push(count - i);
    }

    for (int i = 0; i < count; i++) {
      pq.pop();
    }

    x.push_back(count);
    y.push_back(tm.timeus());
    std::cout << " time (us): " << tm.timeus() << "\n";
  }
}
