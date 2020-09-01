// Copyright (C) 2019-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Tests for calculating offset for reference timestamps.
///
//===----------------------------------------------------------------------===//

#include <adc_readout/OffsetTime.h>
#include <ctime>
#include <gtest/gtest.h>

TEST(OffsetTimeTest, NoOffset) {
  OffsetTime UnderTest(OffsetTime::NONE);
  EXPECT_EQ(UnderTest.calcTimestampNS(123456789), 123456789u);
}

TEST(OffsetTimeTest, OffsetToNow1) {
  auto Now = std::chrono::system_clock::now();
  auto NowNS = std::chrono::duration_cast<std::chrono::nanoseconds>(
                   Now.time_since_epoch())
                   .count();
  std::uint64_t SomeTimeStamp{NowNS + 1000000000ull};
  OffsetTime UnderTest(OffsetTime::NOW, {}, SomeTimeStamp);
  EXPECT_NEAR(UnderTest.calcTimestampNS(SomeTimeStamp),
              static_cast<std::uint64_t>(NowNS), 1e6);
}

TEST(OffsetTimeTest, OffsetToNow2) {
  auto Now = std::chrono::system_clock::now();
  auto NowNS = std::chrono::duration_cast<std::chrono::nanoseconds>(
                   Now.time_since_epoch())
                   .count();
  std::uint64_t SomeTimeStamp{NowNS - 1000000000ull};
  OffsetTime UnderTest(OffsetTime::NOW, {}, SomeTimeStamp);
  EXPECT_NEAR(UnderTest.calcTimestampNS(SomeTimeStamp),
              static_cast<std::uint64_t>(NowNS), 1e6);
}

TEST(OffsetTimeTest, OffsetToTimePoint1) {
  auto SomeTimeStamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count();
  std::tm t;
  std::string DateTimeString("1996-02-25 12:13:14");
  std::istringstream InSS(DateTimeString);
  InSS >> std::get_time(&t, "%Y-%m-%d %H:%M:%S");

  auto TestTimePoint = std::chrono::system_clock::from_time_t(std::mktime(&t));

  OffsetTime UnderTest(OffsetTime::TIME_POINT, TestTimePoint, SomeTimeStamp);
  auto TestNS = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    TestTimePoint.time_since_epoch())
                    .count();
  EXPECT_EQ(UnderTest.calcTimestampNS(SomeTimeStamp),
            static_cast<std::uint64_t>(TestNS));
}

TEST(OffsetTimeTest, OffsetToTimePoint2) {
  auto SomeTimeStamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count();
  std::tm t;
  std::string DateTimeString("2026-02-25 12:13:14");
  std::istringstream InSS(DateTimeString);
  InSS >> std::get_time(&t, "%Y-%m-%d %H:%M:%S");

  auto TestTimePoint = std::chrono::system_clock::from_time_t(std::mktime(&t));

  OffsetTime UnderTest(OffsetTime::TIME_POINT, TestTimePoint, SomeTimeStamp);
  auto TestNS = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    TestTimePoint.time_since_epoch())
                    .count();
  EXPECT_EQ(UnderTest.calcTimestampNS(SomeTimeStamp),
            static_cast<std::uint64_t>(TestNS));
}
