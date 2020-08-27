/** Copyright (C) 2018-2020 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include <adc_readout/DelayLineProcessing.h>
#include <algorithm>
#include <cmath>
#include <gtest/gtest.h>

class SampleRunAnalysis : public ::testing::Test {
public:
  void SetUp() override {
    TestData.Data.resize(10, 0);
    TestData.Identifier = {SourceID, ChannelNr};
  }
  const unsigned short ChannelNr{42};
  const unsigned short SourceID{84};
  SamplingRun TestData;
};

TEST_F(SampleRunAnalysis, MaxValueTest) {
  auto MaxPosition = 5;
  auto MaxValue = 10;
  double Threshold{0.1};
  TestData.Data[MaxPosition] = MaxValue;
  auto Result = analyseSampleRun(TestData, Threshold);
  EXPECT_EQ(Result.BackgroundLevel, 0);
  EXPECT_EQ(Result.PeakLevel, MaxValue);
  EXPECT_EQ(Result.PeakAmplitude, MaxValue);
  EXPECT_EQ(Result.PeakArea, MaxValue);
  EXPECT_EQ(Result.PeakTimestamp,
            RawTimeStamp(0, MaxPosition * TestData.OversamplingFactor));
  EXPECT_EQ(Result.ThresholdTimestamp, RawTimeStamp(0, MaxPosition));
  EXPECT_EQ(Result.Identifier.SourceID, SourceID);
  EXPECT_EQ(Result.Identifier.ChannelNr, ChannelNr);
}

TEST_F(SampleRunAnalysis, BackgroundTest) {
  double Threshold{0.1};
  auto MaxPosition = 5u;
  auto MaxValue = 10;
  auto StartBkg = 1;
  auto EndBkg = 5;
  auto Slope = (EndBkg - StartBkg) / double(TestData.Data.size());
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
  EXPECT_EQ(Result.PeakTimestamp,
            RawTimeStamp(0, MaxPosition * TestData.OversamplingFactor));
  EXPECT_EQ(Result.ThresholdTimestamp, RawTimeStamp(0, MaxPosition));
}

TEST_F(SampleRunAnalysis, BkgAndAreaTest) {
  double Threshold{0.1};
  auto MaxPosition = 5u;
  auto MaxValue = 10;
  auto StartBkg = 1;
  auto EndBkg = 5;
  auto Slope = (EndBkg - StartBkg) / double(TestData.Data.size());
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
  EXPECT_EQ(Result.PeakTimestamp,
            RawTimeStamp(0, MaxPosition * TestData.OversamplingFactor));
  EXPECT_EQ(Result.ThresholdTimestamp, RawTimeStamp(0, MaxPosition));
}

TEST_F(SampleRunAnalysis, ThresholdTest) {
  double Threshold{0.1};
  auto StartPosition = 5;
  auto AddValue = 300;
  std::fill(TestData.Data.begin() + StartPosition, TestData.Data.end() - 1,
            AddValue);
  auto Result = analyseSampleRun(TestData, Threshold);
  EXPECT_EQ(Result.BackgroundLevel, 0);
  EXPECT_EQ(Result.PeakLevel, AddValue);
  EXPECT_EQ(Result.PeakAmplitude, AddValue);
  EXPECT_EQ(Result.PeakTimestamp,
            RawTimeStamp(0, StartPosition * TestData.OversamplingFactor));
  EXPECT_EQ(Result.ThresholdTimestamp,
            RawTimeStamp(0, StartPosition * TestData.OversamplingFactor));
}
