/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Unit tests.
 */

#include <gtest/gtest.h>
#include "../AdcDataProcessor.h"

static const std::uint32_t TimerCounterMax = 88052500/2;

TEST(TimeStampCalcTest, CalcSeconds) {
  std::uint32_t Seconds = 1;
  std::uint64_t TimeStamp = TimeStamp::Calc(Seconds, 0);
  EXPECT_EQ(TimeStamp, static_cast<std::uint64_t>(Seconds) * 1000000000);
}

TEST(TimeStampCalcTest, CalcNanoSec) {
  std::uint32_t SecondsFrac = 1;
  std::uint64_t TimeStamp = TimeStamp::Calc(0, SecondsFrac);
  std::uint64_t TestTimeStamp = (double(SecondsFrac) / static_cast<double>(TimerCounterMax)) * 1e9 + 0.5;
  EXPECT_EQ(TimeStamp, static_cast<std::uint64_t>(TestTimeStamp));
}

TEST(TimeStampCalcTest, Comparison) {
  for (unsigned int i = 0; i < TimerCounterMax; ++i) {
    ASSERT_EQ(TimeStamp::Calc(0, i), TimeStamp::CalcFast(0, i)) << "Failed with input: " << i;
  }
}

TEST(TimeStampCalcTest, Sample1) {
  std::uint32_t Sec = 54;
  std::uint32_t SecFrac = TimerCounterMax;
  std::uint32_t SampleNr = 0;
  EXPECT_EQ(TimeStamp::CalcSample(Sec, SecFrac, SampleNr), TimeStamp::Calc(Sec + 1, 0));
}

TEST(TimeStampCalcTest, Sample2) {
  std::uint32_t Sec = 54;
  std::uint32_t SecFrac = TimerCounterMax - 5;
  std::uint32_t SampleNr = 10;
  EXPECT_EQ(TimeStamp::CalcSample(Sec, SecFrac, SampleNr), TimeStamp::Calc(Sec + 1, 5));
}

TEST(TimeStampCalcTest, Sample3) {
  std::uint32_t Sec = 54;
  std::uint32_t SecFrac = 0;
  std::uint32_t SampleNr = 0;
  EXPECT_EQ(TimeStamp::CalcSample(Sec, SecFrac, SampleNr), TimeStamp::Calc(Sec, 0));
}

TEST(TimeStampCalcTest, Sample4) {
  std::uint32_t Sec = 54;
  std::uint32_t SecFrac = 100;
  std::uint32_t SampleNr = 50;
  EXPECT_EQ(TimeStamp::CalcSample(Sec, SecFrac, SampleNr), TimeStamp::Calc(Sec, SecFrac + SampleNr));
}

