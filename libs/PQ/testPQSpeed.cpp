#include <Timer.h>
#include <cstdlib>
#include <functional>
#include <gtest/gtest.h>
#include <iostream>
#include <queue>

class PriorityQueueTest : public ::testing::Test {
protected:
  virtual void SetUp() { std::srand(4242); }

  // virtual void TearDown() {}

  std::priority_queue<int, std::vector<int>, std::greater<int>> pq;
  std::vector<int> pqsizes{250000,  500000,  750000,  1000000, 1500000,
                           2000000, 3000000, 4000000, 5000000};
  Timer tm;
  int random_variable;
};

TEST_F(PriorityQueueTest, DoublingTestRandom) {
  for (auto const &count : pqsizes) {
    std::cout << "inserting " << count << " random ints -";

    tm.Start();
    for (int i = 0; i < count; i++) {
      random_variable = std::rand();
      pq.push(random_variable);
    }

    for (int i = 0; i < count; i++) {
      pq.pop();
    }
    tm.Stop();
    std::cout << " time (us): " << tm.ElapsedUS() << "\n";
  }
}

TEST_F(PriorityQueueTest, RandomSpeed) {
  for (auto const &count : pqsizes) {
    std::cout << "generating " << count << " random ints -";

    tm.Start();
    for (int i = 0; i < count; i++) {
      int random_variable = std::rand();
    }
    tm.Stop();
    std::cout << " time (us): " << tm.ElapsedUS() << "\n";
  }
}

TEST_F(PriorityQueueTest, AlreadySortedIncreasing) {
  for (auto const &count : pqsizes) {
    std::cout << "generating " << count << " sorted ints (increasing) -";

    tm.Start();
    for (int i = 0; i < count; i++) {
      pq.push(i);
    }

    for (int i = 0; i < count; i++) {
      pq.pop();
    }
    tm.Stop();

    std::cout << " time (us): " << tm.ElapsedUS() << "\n";
  }
}

TEST_F(PriorityQueueTest, AlreadySortedDecreasing) {
  for (auto const &count : pqsizes) {
    std::cout << "generating " << count << " sorted ints (decreasing)-";

    tm.Start();
    for (int i = 0; i < count; i++) {
      pq.push(count - i);
    }

    for (int i = 0; i < count; i++) {
      pq.pop();
    }
    tm.Stop();

    std::cout << " time (us): " << tm.ElapsedUS() << "\n";
  }
}
