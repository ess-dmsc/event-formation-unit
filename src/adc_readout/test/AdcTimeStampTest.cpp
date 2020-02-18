/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Testing of time stamp calculations.
 */

#include "../AdcTimeStamp.h"
#include "../AdcReadoutConstants.h"
#include <cmath>
#include <ctime>
#include <gtest/gtest.h>
#include <random>

static auto const ExtClk = TimeStamp::ClockMode::External;
static auto const IntClk = TimeStamp::ClockMode::Internal;

TEST(TimeStampCalcTest, CalcSeconds) {
  std::uint32_t Seconds{1};
  TimeStamp TS{{Seconds, 0}, ExtClk};
  EXPECT_EQ(TS.getTimeStampNS(),
            static_cast<std::uint64_t>(Seconds) * 1000000000);
}

TEST(TimeStampCalcTest, CalcNanoSec) {
  std::uint32_t SecondsFrac = 1;
  TimeStamp TS{{0, SecondsFrac}, ExtClk};
  std::uint64_t TestTimeStamp =
      std::llround((double(SecondsFrac) /
                    static_cast<double>(TimerClockFrequencyExternal / 2)) *
                   1e9);
  EXPECT_EQ(TS.getTimeStampNS(), static_cast<std::uint64_t>(TestTimeStamp));
}

TEST(TimeStampCalcTest, Sample1) {
  std::uint32_t Sec = 54;
  std::uint32_t SecFrac = TimerClockFrequencyExternal / 2;
  std::uint32_t SampleNr = 0;
  TimeStamp TS{{Sec, SecFrac}, ExtClk};
  EXPECT_EQ(TS.getOffsetTimeStamp(SampleNr).getTimeStampNS(),
            (TimeStamp{{Sec + 1, 0}, ExtClk}).getTimeStampNS());
}

TEST(TimeStampCalcTest, ComparisonExternalInternalFail) {
  std::uint32_t Sec = 54;
  std::uint32_t SecFrac = 125;
  TimeStamp TS1{{Sec, SecFrac}, ExtClk};
  TimeStamp TS2{{Sec, SecFrac}, IntClk};
  EXPECT_NE(TS1.getTimeStampNS(), TS2.getTimeStampNS());
}

TEST(TimeStampCalcTest, Sample2) {
  std::uint32_t Sec = 54;
  std::uint32_t SecFrac = (TimerClockFrequencyExternal / 2) - 5;
  std::uint32_t SampleNr = 10;
  TimeStamp TS1{{Sec, SecFrac}, TimeStamp::ClockMode::External};
  TimeStamp TS2{{Sec + 1, 5}, TimeStamp::ClockMode::External};
  EXPECT_EQ(TS1.getOffsetTimeStamp(SampleNr).getTimeStampNS(),
            TS2.getTimeStampNS());
}

TEST(TimeStampCalcTest, Sample3) {
  std::uint32_t Sec = 54;
  std::uint32_t SecFrac = 0;
  std::uint32_t SampleNr = 0;
  TimeStamp TS1{{Sec, SecFrac}, TimeStamp::ClockMode::External};
  TimeStamp TS2{{Sec, 0}, TimeStamp::ClockMode::External};
  EXPECT_EQ(TS1.getOffsetTimeStamp(SampleNr).getTimeStampNS(),
            TS2.getTimeStampNS());
}

TEST(TimeStampCalcTest, Sample4) {
  std::uint32_t Sec = 54;
  std::uint32_t SecFrac = 100;
  std::uint32_t SampleNr = 50;
  TimeStamp TS1{{Sec, SecFrac}, ExtClk};
  TimeStamp TS2{{Sec, SecFrac + SampleNr}, ExtClk};
  EXPECT_EQ(TS1.getOffsetTimeStamp(SampleNr).getTimeStampNS(),
            TS2.getTimeStampNS());
}

TEST(TimeStampCalcTest, Sample5) {
  std::uint32_t Sec = 54;
  std::uint32_t SecFrac = 100;
  std::int32_t SampleNr = -50;
  TimeStamp TS1{{Sec, SecFrac}, ExtClk};
  TimeStamp TS2{{Sec, SecFrac + SampleNr}, ExtClk};
  EXPECT_EQ(TS1.getOffsetTimeStamp(SampleNr).getTimeStampNS(),
            TS2.getTimeStampNS());
}

TEST(TimeStampCalcTest, Sample6) {
  std::uint32_t Sec = 54;
  std::uint32_t SecFrac = 10;
  std::int32_t SampleNr = -50;
  TimeStamp TS1{{Sec, SecFrac}, ExtClk};
  TimeStamp TS2{
      {Sec - 1, (TimerClockFrequencyExternal / 2) + SecFrac + SampleNr},
      ExtClk};
  EXPECT_EQ(TS1.getOffsetTimeStamp(SampleNr).getTimeStampNS(),
            TS2.getTimeStampNS());
}

TEST(TimeStampCalcTest, EpochTime1) {
  TimeStamp TS{5ull * 1000000000ull, ExtClk};
  EXPECT_EQ(TS.getSeconds(), 5u);
  EXPECT_EQ(TS.getSecondsFrac(), 0u);
}

TEST(TimeStampCalcTest, EpochTime2) {
  TimeStamp TS{5ull * 1000000000ull + 5ull, ExtClk};
  EXPECT_EQ(TS.getSeconds(), 5u);
  EXPECT_EQ(TS.getSecondsFrac(), 0u);
}

TEST(TimeStampCalcTest, EpochTime3) {
  TimeStamp TS{4ull * 1000000000ull + 999999999ull, ExtClk};
  EXPECT_EQ(TS.getSeconds(), 5u);
  EXPECT_EQ(TS.getSecondsFrac(), 0u);
}

TEST(TimeStampCalcTest, EpochTime4) {
  time_t UnixEpoch{0};
  time(&UnixEpoch);
  TimeStamp TS{UnixEpoch * 1000000000ull, ExtClk};
  EXPECT_EQ(TS.getSeconds(), UnixEpoch);
  EXPECT_EQ(TS.getSecondsFrac(), 0u);
}

TEST(TimeStampCalcTest, EpochTime5) {
  time_t UnixEpoch{0};
  time(&UnixEpoch);
  TimeStamp TS{UnixEpoch * 1000000000ull + 5ull, ExtClk};
  EXPECT_EQ(TS.getSeconds(), UnixEpoch);
  EXPECT_EQ(TS.getSecondsFrac(), 0u);
}

TEST(TimeStampCalcTest, EpochTime6) {
  time_t UnixEpoch{0};
  time(&UnixEpoch);
  TimeStamp TS{(UnixEpoch - 1) * 1000000000ull + 999999999ull, ExtClk};
  EXPECT_EQ(TS.getSeconds(), UnixEpoch);
  EXPECT_EQ(TS.getSecondsFrac(), 0u);
}

TEST(TimeStampCalcTest, EpochTime7) {
  time_t UnixEpoch{0};
  time(&UnixEpoch);
  TimeStamp TS{UnixEpoch * 1000000000ull + 25ull, ExtClk};
  EXPECT_EQ(TS.getSeconds(), UnixEpoch);
  EXPECT_EQ(TS.getSecondsFrac(), 1u);
}

TEST(TimeStampCalcTest, EpochTime8) {
  time_t UnixEpoch{0};
  time(&UnixEpoch);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 1000000000);

  for (int u = 0; u < 10000; ++u) {
    std::uint64_t const TestTime = UnixEpoch * 1000000000ull + dis(gen);
    TimeStamp TS{TestTime, ExtClk};
    ASSERT_LT(std::abs(static_cast<int64_t>(TestTime) -
                       static_cast<int64_t>(TS.getTimeStampNS())),
              12);
  }
}