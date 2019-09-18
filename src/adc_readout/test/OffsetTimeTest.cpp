// Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Tests for calculating offset for reference timestamps.
///
//===----------------------------------------------------------------------===//

#include "../OffsetTime.h"
#include <gtest/gtest.h>
#include <ctime>

TEST(OffsetTimeTest, NoOffset) {
  OffsetTime UnderTest(OffsetTime::NONE);
  EXPECT_EQ(UnderTest.calcTimestamp(123456789), 123456789u);
}

TEST(OffsetTimeTest, OffsetToNow) {
  OffsetTime UnderTest(OffsetTime::NOW);
  auto Now = std::chrono::system_clock::now();
  auto NowNS = std::chrono::duration_cast<std::chrono::nanoseconds>(Now.time_since_epoch()).count();
  EXPECT_NEAR(UnderTest.calcTimestamp(123456789), NowNS, 1e8);
}

TEST(OffsetTimeTest, OffsetToTimePoint1) {
  OffsetTime UnderTest(OffsetTime::TIME_POINT);
  auto Now = std::chrono::system_clock::now();
  auto NowNS = std::chrono::duration_cast<std::chrono::nanoseconds>(Now.time_since_epoch()).count();
  EXPECT_NEAR(UnderTest.calcTimestamp(123456789), NowNS, 1e8);
}

TEST(OffsetTimeTest, OffsetToTimePoint2) {
  std::tm t;
  std::string DateTimeString("1996-02-25 12:13:14");
  std::istringstream InSS(DateTimeString);
  InSS >> std::get_time(&t, "%Y-%m-%d %H:%M:%S");

  auto TestTimePoint = std::chrono::system_clock::from_time_t(std::mktime(&t));

  OffsetTime UnderTest(OffsetTime::TIME_POINT, TestTimePoint);
  auto TestNS = std::chrono::duration_cast<std::chrono::nanoseconds>(TestTimePoint.time_since_epoch()).count();
  auto CalculatedTS = UnderTest.calcTimestamp(123456789);
  EXPECT_NEAR(CalculatedTS, TestNS, 1e8);
}

TEST(OffsetTimeTest, OffsetToTimePoint3) {
  std::tm t;
  std::string DateTimeString("1996-02-25 12:13:14");
  std::istringstream InSS(DateTimeString);
  InSS >> std::get_time(&t, "%Y-%m-%d %H:%M:%S");

  auto TestTimePoint = std::chrono::system_clock::from_time_t(std::mktime(&t));

  OffsetTime UnderTest(TestTimePoint);
  auto TestNS = std::chrono::duration_cast<std::chrono::nanoseconds>(TestTimePoint.time_since_epoch()).count();
  auto CalculatedTS = UnderTest.calcTimestamp(123456789);
  EXPECT_NEAR(CalculatedTS, TestNS, 1e8);
}