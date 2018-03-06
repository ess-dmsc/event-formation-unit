/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Testing of time stamp calculations.
 */

#include <gtest/gtest.h>
#include "../AdcTimeStamp.h"

static const std::uint32_t TimerCounterMax = 88052500/2;

TEST(TimeStampCalcTest, CalcSeconds) {
  std::uint32_t Seconds = 1;
  RawTimeStamp TS{Seconds, 0};
  EXPECT_EQ(TS.GetTimeStampNS(), static_cast<std::uint64_t>(Seconds) * 1000000000);
}

TEST(TimeStampCalcTest, CalcNanoSec) {
  std::uint32_t SecondsFrac = 1;
  RawTimeStamp TS{0, SecondsFrac};
  std::uint64_t TestTimeStamp = (double(SecondsFrac) / static_cast<double>(TimerCounterMax)) * 1e9 + 0.5;
  EXPECT_EQ(TS.GetTimeStampNS(), static_cast<std::uint64_t>(TestTimeStamp));
}

TEST(TimeStampCalcTest, Comparison) {
  for (unsigned int i = 0; i < TimerCounterMax; ++i) {
    RawTimeStamp TS{0, i};
    ASSERT_EQ(TS.GetTimeStampNS(), TS.GetTimeStampNSFast()) << "Failed with input: " << i;
  }
}

TEST(TimeStampCalcTest, Sample1) {
  std::uint32_t Sec = 54;
  std::uint32_t SecFrac = TimerCounterMax;
  std::uint32_t SampleNr = 0;
  RawTimeStamp TS{Sec, SecFrac};
  EXPECT_EQ(TS.GetOffsetTimeStamp(SampleNr).GetTimeStampNS(), (RawTimeStamp{Sec + 1, 0}).GetTimeStampNS());
}

TEST(TimeStampCalcTest, Sample2) {
  std::uint32_t Sec = 54;
  std::uint32_t SecFrac = TimerCounterMax - 5;
  std::uint32_t SampleNr = 10;
  RawTimeStamp TS1{Sec, SecFrac};
  RawTimeStamp TS2{Sec + 1, 5};
  EXPECT_EQ(TS1.GetOffsetTimeStamp(SampleNr).GetTimeStampNS(), TS2.GetTimeStampNS());
}

TEST(TimeStampCalcTest, Sample3) {
  std::uint32_t Sec = 54;
  std::uint32_t SecFrac = 0;
  std::uint32_t SampleNr = 0;
  RawTimeStamp TS1{Sec, SecFrac};
  RawTimeStamp TS2{Sec, 0};
  EXPECT_EQ(TS1.GetOffsetTimeStamp(SampleNr).GetTimeStampNS(), TS2.GetTimeStampNS());
}

TEST(TimeStampCalcTest, Sample4) {
  std::uint32_t Sec = 54;
  std::uint32_t SecFrac = 100;
  std::uint32_t SampleNr = 50;
  RawTimeStamp TS1{Sec, SecFrac};
  RawTimeStamp TS2{Sec, SecFrac + SampleNr};
  EXPECT_EQ(TS1.GetOffsetTimeStamp(SampleNr).GetTimeStampNS(), TS2.GetTimeStampNS());
}

TEST(TimeStampCalcTest, Sample5) {
  std::uint32_t Sec = 54;
  std::uint32_t SecFrac = 100;
  std::int32_t SampleNr = -50;
  RawTimeStamp TS1{Sec, SecFrac};
  RawTimeStamp TS2{Sec, SecFrac + SampleNr};
  EXPECT_EQ(TS1.GetOffsetTimeStamp(SampleNr).GetTimeStampNS(), TS2.GetTimeStampNS());
}

TEST(TimeStampCalcTest, Sample6) {
  std::uint32_t Sec = 54;
  std::uint32_t SecFrac = 10;
  std::int32_t SampleNr = -50;
  RawTimeStamp TS1{Sec, SecFrac};
  RawTimeStamp TS2{Sec - 1, TimerCounterMax + SecFrac + SampleNr};
  EXPECT_EQ(TS1.GetOffsetTimeStamp(SampleNr).GetTimeStampNS(), TS2.GetTimeStampNS());
}

