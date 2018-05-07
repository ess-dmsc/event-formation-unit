/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Unit tests.
 */

#include <gtest/gtest.h>
#include "../PeakFinder.h"

TEST(PeakFinderTest, GetMaxValue) {
  std::uint16_t NormalValue = 500;
  std::vector<std::uint16_t> TestData(100, NormalValue);
  int MaxLoc = 45;
  std::uint16_t MaxValue = 750;
  TestData[MaxLoc] = MaxValue;
  auto Results = FindPeak(TestData);
  EXPECT_EQ(MaxValue, Results.Max);
  EXPECT_EQ(MaxLoc, Results.MaxLocation);
}

TEST(PeakFinderTest, GetMinValue) {
  std::uint16_t NormalValue = 500;
  std::vector<std::uint16_t> TestData(100, NormalValue);
  int MinLoc = 85;
  std::uint16_t MinValue = 5;
  TestData[MinLoc] = MinValue;
  auto Results = FindPeak(TestData);
  
  EXPECT_EQ(NormalValue, Results.Max);
  EXPECT_EQ(0, Results.MaxLocation);
}

TEST(PeakFinderTest, GetMeanValue) {
  std::uint16_t NormalValue = 500;
  std::vector<std::uint16_t> TestData(100, NormalValue);
  auto Results = FindPeak(TestData);
  EXPECT_EQ(NormalValue, Results.Mean);
}

