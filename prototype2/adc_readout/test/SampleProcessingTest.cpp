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
private:
  
};

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
