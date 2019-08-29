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

TEST(TimeStampCalcTest, CalcSeconds) {
  std::uint32_t Seconds{1};
  RawTimeStamp TS{Seconds, 0};
  EXPECT_EQ(TS.getTimeStampNS(),
            static_cast<std::uint64_t>(Seconds) * 1000000000);
}

TEST(TimeStampCalcTest, CalcNanoSec) {
  std::uint32_t SecondsFrac = 1;
  RawTimeStamp TS{0, SecondsFrac};
  std::uint64_t TestTimeStamp = std::llround(
      (double(SecondsFrac) / static_cast<double>(AdcTimerCounterMax)) * 1e9);
  EXPECT_EQ(TS.getTimeStampNS(), static_cast<std::uint64_t>(TestTimeStamp));
}

TEST(TimeStampCalcTest, Comparison) {
  for (unsigned int i = 0; i < AdcTimerCounterMax; ++i) {
    RawTimeStamp TS{0, i};
    ASSERT_EQ(TS.getTimeStampNS(), TS.getTimeStampNSFast())
        << "Failed with input: " << i;
  }
}

TEST(TimeStampCalcTest, Sample1) {
  std::uint32_t Sec = 54;
  std::uint32_t SecFrac = AdcTimerCounterMax;
  std::uint32_t SampleNr = 0;
  RawTimeStamp TS{Sec, SecFrac};
  EXPECT_EQ(TS.getOffsetTimeStamp(SampleNr).getTimeStampNS(),
            (RawTimeStamp{Sec + 1, 0}).getTimeStampNS());
}

TEST(TimeStampCalcTest, Sample2) {
  std::uint32_t Sec = 54;
  std::uint32_t SecFrac = AdcTimerCounterMax - 5;
  std::uint32_t SampleNr = 10;
  RawTimeStamp TS1{Sec, SecFrac};
  RawTimeStamp TS2{Sec + 1, 5};
  EXPECT_EQ(TS1.getOffsetTimeStamp(SampleNr).getTimeStampNS(),
            TS2.getTimeStampNS());
}

TEST(TimeStampCalcTest, Sample3) {
  std::uint32_t Sec = 54;
  std::uint32_t SecFrac = 0;
  std::uint32_t SampleNr = 0;
  RawTimeStamp TS1{Sec, SecFrac};
  RawTimeStamp TS2{Sec, 0};
  EXPECT_EQ(TS1.getOffsetTimeStamp(SampleNr).getTimeStampNS(),
            TS2.getTimeStampNS());
}

TEST(TimeStampCalcTest, Sample4) {
  std::uint32_t Sec = 54;
  std::uint32_t SecFrac = 100;
  std::uint32_t SampleNr = 50;
  RawTimeStamp TS1{Sec, SecFrac};
  RawTimeStamp TS2{Sec, SecFrac + SampleNr};
  EXPECT_EQ(TS1.getOffsetTimeStamp(SampleNr).getTimeStampNS(),
            TS2.getTimeStampNS());
}

TEST(TimeStampCalcTest, Sample5) {
  std::uint32_t Sec = 54;
  std::uint32_t SecFrac = 100;
  std::int32_t SampleNr = -50;
  RawTimeStamp TS1{Sec, SecFrac};
  RawTimeStamp TS2{Sec, SecFrac + SampleNr};
  EXPECT_EQ(TS1.getOffsetTimeStamp(SampleNr).getTimeStampNS(),
            TS2.getTimeStampNS());
}

TEST(TimeStampCalcTest, Sample6) {
  std::uint32_t Sec = 54;
  std::uint32_t SecFrac = 10;
  std::int32_t SampleNr = -50;
  RawTimeStamp TS1{Sec, SecFrac};
  RawTimeStamp TS2{Sec - 1, AdcTimerCounterMax + SecFrac + SampleNr};
  EXPECT_EQ(TS1.getOffsetTimeStamp(SampleNr).getTimeStampNS(),
            TS2.getTimeStampNS());
}

TEST(TimeStampCalcTest, EpochTime1) {
  RawTimeStamp TS{5ull * 1000000000ull};
  EXPECT_EQ(TS.Seconds, 5u);
  EXPECT_EQ(TS.SecondsFrac, 0u);
}

TEST(TimeStampCalcTest, EpochTime2) {
  RawTimeStamp TS{5ull * 1000000000ull + 5ull};
  EXPECT_EQ(TS.Seconds, 5u);
  EXPECT_EQ(TS.SecondsFrac, 0u);
}

TEST(TimeStampCalcTest, EpochTime3) {
  RawTimeStamp TS{4ull * 1000000000ull + 999999999ull};
  EXPECT_EQ(TS.Seconds, 5u);
  EXPECT_EQ(TS.SecondsFrac, 0u);
}

TEST(TimeStampCalcTest, EpochTime4) {
  time_t UnixEpoch{0};
  time(&UnixEpoch);
  RawTimeStamp TS{UnixEpoch * 1000000000ull};
  EXPECT_EQ(TS.Seconds, UnixEpoch);
  EXPECT_EQ(TS.SecondsFrac, 0u);
}

TEST(TimeStampCalcTest, EpochTime5) {
  time_t UnixEpoch{0};
  time(&UnixEpoch);
  RawTimeStamp TS{UnixEpoch * 1000000000ull + 5ull};
  EXPECT_EQ(TS.Seconds, UnixEpoch);
  EXPECT_EQ(TS.SecondsFrac, 0u);
}

TEST(TimeStampCalcTest, EpochTime6) {
  time_t UnixEpoch{0};
  time(&UnixEpoch);
  RawTimeStamp TS{(UnixEpoch - 1) * 1000000000ull + 999999999ull};
  EXPECT_EQ(TS.Seconds, UnixEpoch);
  EXPECT_EQ(TS.SecondsFrac, 0u);
}

TEST(TimeStampCalcTest, EpochTime7) {
  time_t UnixEpoch{0};
  time(&UnixEpoch);
  RawTimeStamp TS{UnixEpoch * 1000000000ull + 25ull};
  EXPECT_EQ(TS.Seconds, UnixEpoch);
  EXPECT_EQ(TS.SecondsFrac, 1u);
}

TEST(TimeStampCalcTest, EpochTime8) {
  time_t UnixEpoch{0};
  time(&UnixEpoch);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 1000000000);

  for (int u = 0; u < 10000; ++u) {
    std::uint64_t const TestTime = UnixEpoch * 1000000000ull + dis(gen);
    RawTimeStamp TS{TestTime};
    ASSERT_LT(std::abs(static_cast<int64_t>(TestTime) -
                       static_cast<int64_t>(TS.getTimeStampNS())),
              12);
  }
}