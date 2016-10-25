/** Copyright (C) 2016 European Spallation Source */

#include <NMXEvent.h>
#include <Timer.h>
#include <cstdlib>
#include <functional>
#include <gtest/gtest.h>
#include <iostream>
#include <queue>

TEST(Event, BasicTests) {

  NMXData eva(100, 200, 300);
  NMXData evb(101, 200, 300);

  ASSERT_EQ(100, eva.getdetector());
  ASSERT_EQ(200, eva.gettime());
  ASSERT_EQ(300, eva.getadc());

  ASSERT_EQ(true, eva < evb);
  ASSERT_EQ(false, evb < eva);
}

TEST(Event, AllocationTime) {
  srand(4242);
  std::vector<int> pqsizes{250000,  500000,  750000,  1000000, 1500000,
                           2000000, 3000000, 4000000, 5000000};

  for (auto const &size : pqsizes) {
    Timer tm;
    std::priority_queue<NMXData, std::vector<NMXData>, std::greater<NMXData>>
        pq;

    for (int i = 0; i < size; i++) {
      auto ev = new NMXData(random(), size, 1);
      pq.push(*ev);
    }
    for (int i = 0; i < size; i++) {
      pq.pop();
    }
    std::cout << size << " constructors " << tm.timeus() << " us\n";
  }
}
