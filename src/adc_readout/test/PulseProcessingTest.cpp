/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include "../AdcReadoutConstants.h"
#include "../DelayLineProcessing.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <gtest/gtest.h>
#include <random>

class SampleRunAnalysis : public ::testing::Test {
public:
  void SetUp() override {
    TestData.Data.resize(10, 0);
    TestData.Identifier = {SourceID, ChannelNr};
  }
  const std::uint16_t ChannelNr{42};
  const std::uint16_t SourceID{84};
  SamplingRun TestData;
};

TEST_F(SampleRunAnalysis, MaxValueTest) {
  std::uint32_t MaxPosition = 5;
  auto MaxValue = 10;
  double Threshold{0.1};
  TestData.Data[MaxPosition] = MaxValue;
  auto Result = analyseSampleRun(TestData, Threshold);
  EXPECT_EQ(Result.BackgroundLevel, 0);
  EXPECT_EQ(Result.PeakLevel, MaxValue);
  EXPECT_EQ(Result.PeakAmplitude, MaxValue);
  EXPECT_EQ(Result.PeakArea, MaxValue);
  EXPECT_EQ(Result.PeakTime,
            TimeStamp({0, MaxPosition * TestData.OversamplingFactor}, TimeStamp::ClockMode::External));
  EXPECT_EQ(Result.ThresholdTime, TimeStamp({0, MaxPosition - 1}, TimeStamp::ClockMode::External));
  EXPECT_EQ(Result.Identifier.SourceID, SourceID);
  EXPECT_EQ(Result.Identifier.ChannelNr, ChannelNr);
}

static auto const ExtClk = TimeStamp::ClockMode::External;

TEST_F(SampleRunAnalysis, BackgroundTest) {
  double Threshold{0.1};
  auto MaxPosition = 5u;
  auto MaxValue = 10;
  auto StartBkg = 1;
  auto EndBkg = 5;
  auto Slope = (EndBkg - StartBkg) / double(TestData.Data.size() - 1);
  for (auto y = 0u; y < TestData.Data.size(); y++) {
    TestData.Data[y] = std::round(StartBkg + Slope * y);
    if (y == MaxPosition) {
      TestData.Data[y] += MaxValue;
    }
  }
  auto Result = analyseSampleRun(TestData, Threshold);
  EXPECT_EQ(Result.BackgroundLevel, int(MaxPosition * Slope + StartBkg));
  EXPECT_EQ(Result.PeakLevel,
            MaxValue + std::round(Slope * MaxPosition) + StartBkg);
  EXPECT_EQ(Result.PeakAmplitude, MaxValue);
  EXPECT_EQ(Result.PeakArea, MaxValue);
  EXPECT_EQ(Result.PeakTime,
            TimeStamp({0, MaxPosition * TestData.OversamplingFactor}, ExtClk));
  EXPECT_EQ(Result.ThresholdTime, TimeStamp({0, MaxPosition - 1}, ExtClk));
}

TEST_F(SampleRunAnalysis, BkgAndAreaTest) {
  double Threshold{0.1};
  auto MaxPosition = 5u;
  auto MaxValue = 10;
  auto StartBkg = 1;
  auto EndBkg = 5;
  auto Slope = (EndBkg - StartBkg) / double(TestData.Data.size() - 1);
  for (auto y = 0u; y < TestData.Data.size(); y++) {
    TestData.Data[y] = std::round(StartBkg + Slope * y);
    if (y == MaxPosition or y == MaxPosition + 1) {
      TestData.Data[y] += MaxValue;
    }
  }
  auto Result = analyseSampleRun(TestData, Threshold);
  EXPECT_EQ(Result.BackgroundLevel, int(MaxPosition * Slope + StartBkg));
  EXPECT_EQ(Result.PeakLevel,
            MaxValue + std::round(Slope * MaxPosition) + StartBkg);
  EXPECT_EQ(Result.PeakAmplitude, MaxValue);
  EXPECT_EQ(Result.PeakArea, MaxValue * 2);
  EXPECT_EQ(Result.PeakTime,
            TimeStamp({0, MaxPosition * TestData.OversamplingFactor}, ExtClk));
  EXPECT_EQ(Result.ThresholdTime, TimeStamp({0, MaxPosition - 1}, ExtClk));
}

TEST_F(SampleRunAnalysis, ThresholdTest1) {
  double Threshold{0.75};
  std::uint32_t StartPosition = 5;
  auto AddValue = 300;
  std::fill(TestData.Data.begin() + StartPosition, TestData.Data.end() - 1,
            AddValue);
  auto Result = analyseSampleRun(TestData, Threshold);
  EXPECT_EQ(Result.BackgroundLevel, 0);
  EXPECT_EQ(Result.PeakLevel, AddValue);
  EXPECT_EQ(Result.PeakAmplitude, AddValue);
  EXPECT_EQ(Result.PeakTime,
            TimeStamp({0, StartPosition * TestData.OversamplingFactor}, ExtClk));
  EXPECT_EQ(Result.ThresholdTime,
            TimeStamp({0, StartPosition * TestData.OversamplingFactor}, ExtClk));
}

TEST_F(SampleRunAnalysis, ThresholdTest2) {
  double Threshold{0.75};
  std::vector<std::uint16_t> Data{0, 1, 2, 4, 0};
  TestData.Data = Data;
  TestData.OversamplingFactor = 1;
  auto Result = analyseSampleRun(TestData, Threshold);
  EXPECT_EQ(Result.ThresholdTime,
            TimeStamp({0, 3u * TestData.OversamplingFactor}, ExtClk));
  EXPECT_NEAR(Result.ThresholdTimestampNS,
              2.5 * Result.PeakTime.getClockCycleLength() * TestData.OversamplingFactor, 1.0);
}

TEST_F(SampleRunAnalysis, ThresholdTest3) {
  double Threshold{0.75};
  std::vector<std::uint16_t> Data{0, 1, 2, 4, 0};
  TestData.Data = Data;
  TestData.OversamplingFactor = 4;
  auto Result = analyseSampleRun(TestData, Threshold);
  EXPECT_EQ(Result.ThresholdTime,
            TimeStamp({0, static_cast<std::uint32_t >(lround(2.5 * TestData.OversamplingFactor))}, ExtClk));
  EXPECT_NEAR(Result.ThresholdTimestampNS,
              2.5 * Result.PeakTime.getClockCycleLength() * TestData.OversamplingFactor, 1.0);
}

TEST_F(SampleRunAnalysis, ThresholdTest4) {
  double Threshold{0.75};
  std::vector<std::uint16_t> Data{1, 2, 3, 5, 1};
  TestData.Data = Data;
  TestData.OversamplingFactor = 4;
  auto Result = analyseSampleRun(TestData, Threshold);
  EXPECT_EQ(Result.ThresholdTime,
            TimeStamp({0, static_cast<std::uint32_t>(lround(2.5 * TestData.OversamplingFactor))}, ExtClk));
  EXPECT_NEAR(Result.ThresholdTimestampNS,
              2.5 * Result.PeakTime.getClockCycleLength() * TestData.OversamplingFactor, 1.0);
}

TEST_F(SampleRunAnalysis, ThresholdTest5) {
  double Threshold{0.75};
  std::vector<std::uint16_t> Data{1, 3, 5, 8, 5};
  TestData.Data = Data;
  TestData.OversamplingFactor = 4;
  auto Result = analyseSampleRun(TestData, Threshold);
  EXPECT_EQ(Result.ThresholdTime,
            TimeStamp({0, static_cast<std::uint32_t>(lround(2.5 * TestData.OversamplingFactor))}, ExtClk));
  EXPECT_NEAR(Result.ThresholdTimestampNS,
              2.5 * Result.PeakTime.getClockCycleLength() * TestData.OversamplingFactor, 1.0);
}

TEST_F(SampleRunAnalysis, ThresholdTest6) {
  double Threshold{0.75};
  std::vector<std::uint16_t> Data{5, 5, 5, 6, 1};
  TestData.Data = Data;
  TestData.OversamplingFactor = 4;
  auto Result = analyseSampleRun(TestData, Threshold);
  EXPECT_EQ(Result.ThresholdTime,
            TimeStamp({0, static_cast<std::uint32_t>(lround(2.5 * TestData.OversamplingFactor))}, ExtClk));
  EXPECT_NEAR(Result.ThresholdTimestampNS,
              2.5 * Result.PeakTime.getClockCycleLength() * TestData.OversamplingFactor, 1.0);
}

TEST_F(SampleRunAnalysis, ThresholdTest7) {
  double Threshold{0.75};
  std::vector<std::uint16_t> Data{5};
  TestData.Data = Data;
  TestData.OversamplingFactor = 4;
  auto Result = analyseSampleRun(TestData, Threshold);
  EXPECT_EQ(Result.ThresholdTime, TimeStamp({0, 0}, ExtClk));
  EXPECT_EQ(Result.ThresholdTimestampNS, 0u);
}

TEST_F(SampleRunAnalysis, ThresholdTest8) {
  double Threshold{0.25};
  std::vector<std::uint16_t> Data{0, 1, 2, 4, 0};
  TestData.Data = Data;
  TestData.OversamplingFactor = 2;
  auto Result = analyseSampleRun(TestData, Threshold);
  EXPECT_EQ(Result.ThresholdTime,
            TimeStamp({0, static_cast<std::uint32_t>(lround(1 * TestData.OversamplingFactor))}, ExtClk));
  EXPECT_NEAR(Result.ThresholdTimestampNS,
              1 * Result.PeakTime.getClockCycleLength() * TestData.OversamplingFactor, 1.0);
}

TEST_F(SampleRunAnalysis, ThresholdTest9) {
  double Threshold{0.375};
  std::vector<std::uint16_t> Data{0, 1, 2, 4, 0};
  TestData.Data = Data;
  TestData.OversamplingFactor = 2;
  auto Result = analyseSampleRun(TestData, Threshold);
  EXPECT_EQ(Result.ThresholdTime,
            TimeStamp({0, static_cast<std::uint32_t >(lround(1.5 * TestData.OversamplingFactor))}, ExtClk));
  EXPECT_NEAR(Result.ThresholdTimestampNS,
              1.5 * Result.PeakTime.getClockCycleLength() * TestData.OversamplingFactor, 1.0);
}

TEST_F(SampleRunAnalysis, ThresholdTest10) {
  double Threshold{0.375};
  std::vector<std::uint16_t> Data{5, 5, 5, 6, 1};
  TestData.Data = Data;
  TestData.OversamplingFactor = 2;
  auto Result = analyseSampleRun(TestData, Threshold);
  EXPECT_EQ(Result.ThresholdTime,
            TimeStamp({0, static_cast<std::uint32_t>(lround(1.5 * TestData.OversamplingFactor))}, ExtClk));
  EXPECT_NEAR(Result.ThresholdTimestampNS,
              1.5 * Result.PeakTime.getClockCycleLength() * TestData.OversamplingFactor, 1.0);
}

TEST_F(SampleRunAnalysis, TimestampTest1) {
  std::vector<std::uint16_t> Data{5, 5, 5, 6, 1};
  TestData.Data = Data;
  TestData.OversamplingFactor = 1;
  auto Seconds = time(nullptr);

  std::random_device RD;
  std::mt19937 RandomGenerator(RD());
  std::uniform_int_distribution<std::uint32_t> RandomDistribution(
      0, TimerClockFrequencyExternal / 2);
  std::uniform_real_distribution<float> ThresholdSelection(0.0, 1.0);
  std::uint32_t SecondsFrac{0};
  for (int y = 0u; y < 1000; ++y) {
    SecondsFrac = RandomDistribution(RandomGenerator);
    TestData.StartTime = {{static_cast<uint32_t>(Seconds),
                          static_cast<uint32_t>(SecondsFrac)}, ExtClk};
    auto Result =
        analyseSampleRun(TestData, ThresholdSelection(RandomGenerator));
    ASSERT_LT(std::abs(static_cast<std::int64_t>(Result.ThresholdTimestampNS) -
                       static_cast<std::int64_t>(
                           Result.ThresholdTime.getTimeStampNS())),
              std::uint32_t(Result.ThresholdTime.getClockCycleLength()));
  }
}
