#include <Timer.h>
#include <cstdlib>
#include <functional>
#include <gtest/gtest.h>
#include <iostream>
#include <queue>

TEST(PriorityQueue, DoublingTestRandom) {
  int random_variable;
  int steps = 6;
  int count = 250000;

  std::srand(4242);

  for (int i = 0; i < steps; i++) {
    std::cout << "inserting " << count << " random ints -";
    std::priority_queue<int, std::vector<int>, std::greater<int>> pq;
    Timer tm;

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
    count += count;
  }
}

TEST(PriorityQueue, RandomSpeed) {
  int steps = 6;
  int count = 250000;

  std::srand(4242);

  for (int i = 0; i < steps; i++) {
    std::cout << "generating " << count << " random ints -";
    Timer tm;

    tm.Start();
    for (int i = 0; i < count; i++) {
      int random_variable = std::rand();
    }
    tm.Stop();
    std::cout << " time (us): " << tm.ElapsedUS() << "\n";
    count += count;
  }
}
