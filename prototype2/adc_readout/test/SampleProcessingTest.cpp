/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Unit tests for the sample processing code (peak extraction and individual samples).
 */

#include <gtest/gtest.h>
#include "../SampleProcessing.h"

class SampleProcessingStandIn : public SampleProcessing {
public:
  SampleProcessingStandIn(std::shared_ptr<Producer> Prod) : SampleProcessing(Prod) {}
  using SampleProcessing::ProcessingInstances;
  using SampleProcessing::MeanOfNrOfSamples;
  using SampleProcessing::TSLocation;
};

TEST(SampleProcessing, InitChannel) {
  std::shared_ptr<Producer> TestProducer(new Producer("None", "None"));
  SampleProcessingStandIn TestProcessor(TestProducer);
}

TEST(SampleProcessingSettingsTest, SetMeanOfSamples) {
  
}

class ChannelProcessingTest : public ::testing::Test {
public:
  virtual void SetUp() override {
    Module.TimeStamp.Seconds = 42;
    Module.TimeStamp.SecondsFrac = 65;
    Module.Channel = 3;
    Module.Data.push_back(1);
    Module.Data.push_back(15);
    Module.Data.push_back(128);
    Module.Data.push_back(0);
  }
  virtual void TearDown() override {
    Module.Data.clear();
  }
  DataModule Module;
};


TEST_F(ChannelProcessingTest, OversamplingAndTime1) {
  ChannelProcessing Processing;
  Processing.setMeanOfSamples(1);
  Processing.setTimeStampLocation(TimeStampLocation::Start);
  auto Result = Processing(Module);
  for (unsigned int i = 0; i < Result.TimeStamps.size(); i++) {
    EXPECT_EQ(Module.TimeStamp.GetOffsetTimeStamp(i).GetTimeStampNS(), Result.TimeStamps.at(i));
  }
}

TEST_F(ChannelProcessingTest, OversamplingAndTime2) {
  ChannelProcessing Processing;
  Processing.setMeanOfSamples(2);
  Processing.setTimeStampLocation(TimeStampLocation::Start);
  auto Result = Processing(Module);
  for (unsigned int i = 0; i < Result.TimeStamps.size(); i++) {
    EXPECT_EQ(Module.TimeStamp.GetOffsetTimeStamp(i * 2).GetTimeStampNS(), Result.TimeStamps.at(i));
  }
}

TEST_F(ChannelProcessingTest, OversamplingAndTime3) {
  ChannelProcessing Processing;
  Processing.setMeanOfSamples(2);
  Processing.setTimeStampLocation(TimeStampLocation::End);
  auto Result = Processing(Module);
  for (unsigned int i = 0; i < Result.TimeStamps.size(); i++) {
    EXPECT_EQ(Module.TimeStamp.GetOffsetTimeStamp(i * 2 + 1).GetTimeStampNS(), Result.TimeStamps.at(i));
  }
}

TEST_F(ChannelProcessingTest, OversamplingAndTime4) {
  ChannelProcessing Processing;
  Module.OversamplingFactor = 4;
  Processing.setMeanOfSamples(1);
  Processing.setTimeStampLocation(TimeStampLocation::Start);
  auto Result = Processing(Module);
  for (unsigned int i = 0; i < Result.TimeStamps.size(); i++) {
    EXPECT_EQ(Module.TimeStamp.GetOffsetTimeStamp(i * 4 - 3).GetTimeStampNS(), Result.TimeStamps.at(i));
  }
}

TEST_F(ChannelProcessingTest, OversamplingAndTime5) {
  ChannelProcessing Processing;
  Module.OversamplingFactor = 4;
  Processing.setMeanOfSamples(1);
  Processing.setTimeStampLocation(TimeStampLocation::End);
  auto Result = Processing(Module);
  for (unsigned int i = 0; i < Result.TimeStamps.size(); i++) {
    EXPECT_EQ(Module.TimeStamp.GetOffsetTimeStamp(i * 4).GetTimeStampNS(), Result.TimeStamps.at(i));
  }
}

TEST_F(ChannelProcessingTest, OversamplingAndTime6) {
  ChannelProcessing Processing;
  Module.OversamplingFactor = 4;
  Processing.setMeanOfSamples(2);
  Processing.setTimeStampLocation(TimeStampLocation::End);
  auto Result = Processing(Module);
  for (unsigned int i = 0; i < Result.TimeStamps.size(); i++) {
    EXPECT_EQ(Module.TimeStamp.GetOffsetTimeStamp(i * 2 * 4 + 4).GetTimeStampNS(), Result.TimeStamps.at(i));
  }
}

TEST_F(ChannelProcessingTest, OversamplingAndTime7) {
  ChannelProcessing Processing;
  Module.OversamplingFactor = 4;
  Processing.setMeanOfSamples(2);
  Processing.setTimeStampLocation(TimeStampLocation::Start);
  auto Result = Processing(Module);
  for (unsigned int i = 0; i < Result.TimeStamps.size(); i++) {
    EXPECT_EQ(Module.TimeStamp.GetOffsetTimeStamp(i * 2 * 4 - 3).GetTimeStampNS(), Result.TimeStamps.at(i));
  }
}

TEST_F(ChannelProcessingTest, OversamplingAndTime8) {
  ChannelProcessing Processing;
  Module.OversamplingFactor = 4;
  Processing.setMeanOfSamples(2);
  Processing.setTimeStampLocation(TimeStampLocation::Middle);
  auto Result = Processing(Module);
  for (unsigned int i = 0; i < Result.TimeStamps.size(); i++) {
    std::uint64_t StartTS = Module.TimeStamp.GetOffsetTimeStamp(i * 2 * 4 - 3).GetTimeStampNS();
    std::uint64_t EndTS = Module.TimeStamp.GetOffsetTimeStamp(i * 2 * 4 + 4).GetTimeStampNS();
    EXPECT_EQ(StartTS + (EndTS - StartTS) / 2, Result.TimeStamps.at(i));
  }
}

TEST_F(ChannelProcessingTest, DefaultSetup) {
  ChannelProcessing Processing;
  auto Result = Processing(Module);
  EXPECT_EQ(Module.Data, Result.Samples);
}

TEST_F(ChannelProcessingTest, Oversampling2X) {
  ChannelProcessing Processing;
  Processing.setMeanOfSamples(2);
  std::vector<std::uint16_t> ExpectedResult{8, 64};
  auto Result = Processing(Module);
  EXPECT_EQ(ExpectedResult, Result.Samples);
}

TEST_F(ChannelProcessingTest, Oversampling3X_OneModule) {
  ChannelProcessing Processing;
  Processing.setMeanOfSamples(3);
  std::vector<std::uint16_t> ExpectedResult{48, };
  auto Result = Processing(Module);
  EXPECT_EQ(ExpectedResult, Result.Samples);
}

TEST_F(ChannelProcessingTest, Oversampling3X_TwoModules) {
  ChannelProcessing Processing;
  Processing.setMeanOfSamples(3);
  Processing(Module);
  auto Result = Processing(Module);
  std::vector<std::uint16_t> ExpectedResult{5, };
  EXPECT_EQ(ExpectedResult, Result.Samples);
}

TEST(CalcTimeStamp, StartTest) {
  RawTimeStamp TS1{53, 500};
  RawTimeStamp TS2{53, 1000};
  EXPECT_EQ(CalcSampleTimeStamp(TS1, TS2, TimeStampLocation::Start), TS1.GetTimeStampNS());
}

TEST(CalcTimeStamp, EndTest) {
  RawTimeStamp TS1{53, 500};
  RawTimeStamp TS2{53, 1000};
  EXPECT_EQ(CalcSampleTimeStamp(TS1, TS2, TimeStampLocation::End), TS2.GetTimeStampNS());
}

TEST(CalcTimeStamp, MiddleTest1) {
  RawTimeStamp TS1{53, 500};
  RawTimeStamp TSMid{53, 750};
  RawTimeStamp TS2{53, 1000};
  EXPECT_EQ(CalcSampleTimeStamp(TS1, TS2, TimeStampLocation::Middle), TSMid.GetTimeStampNS());
}

TEST(CalcTimeStamp, MiddleTest2) {
  RawTimeStamp TS{53, 0};
  RawTimeStamp TSMid{53, 0};
  EXPECT_EQ(CalcSampleTimeStamp(TS.GetOffsetTimeStamp(-150), TS.GetOffsetTimeStamp(150), TimeStampLocation::Middle), TSMid.GetTimeStampNS());
}

TEST(CalcTimeStamp, MiddleTest3) {
  RawTimeStamp TS{53, 1};
  RawTimeStamp TSMid{53, 1};
  EXPECT_EQ(CalcSampleTimeStamp(TS.GetOffsetTimeStamp(-150), TS.GetOffsetTimeStamp(150), TimeStampLocation::Middle), TSMid.GetTimeStampNS());
}

TEST(CalcTimeStamp, MiddleTest4) {
  RawTimeStamp TS{53, 88052500/2 - 5};
  RawTimeStamp TSMid{53, 88052500/2 - 5};
  EXPECT_EQ(CalcSampleTimeStamp(TS.GetOffsetTimeStamp(-150), TS.GetOffsetTimeStamp(150), TimeStampLocation::Middle), TSMid.GetTimeStampNS());
}
