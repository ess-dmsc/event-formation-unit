/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Unit tests for the sample processing code (peak extraction and individual samples).
 */

#include <gtest/gtest.h>
#include "../SampleProcessing.h"
#include <trompeloeil.hpp>

class SampleProcessingStandIn : public SampleProcessing {
public:
  SampleProcessingStandIn(std::shared_ptr<ProducerBase> Prod) : SampleProcessing(Prod) {}
  using SampleProcessing::ProcessingInstances;
  using SampleProcessing::MeanOfNrOfSamples;
  using SampleProcessing::TSLocation;
  MAKE_CONST_MOCK1(serializeAndTransmitData, void(const ProcessedSamples&), override);
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
class ProducerStandIn : public ProducerBase  {
public:
  int produce(char *buffer, int length) override {
    return 0;
  };
};
#pragma GCC diagnostic pop

TEST(SampleProcessing, InitChannel) {
  std::shared_ptr<ProducerBase> TestProducer(new ProducerStandIn());
  SampleProcessingStandIn TestProcessor(TestProducer);
  EXPECT_EQ(TestProcessor.ProcessingInstances.size(), 0);
  PacketData TempPacket;
  DataModule TempModule;
  TempModule.Channel = 1;
  TempPacket.Modules.emplace_back(TempModule);
  TestProcessor(TempPacket);
  EXPECT_EQ(TestProcessor.ProcessingInstances.size(), 1);
  EXPECT_NE(TestProcessor.ProcessingInstances.find(TempModule.Channel), TestProcessor.ProcessingInstances.end());
}

TEST(SampleProcessing, SetMeanOfChannels) {
  std::shared_ptr<ProducerBase> TestProducer(new ProducerStandIn());
  SampleProcessingStandIn TestProcessor(TestProducer);
  PacketData TempPacket;
  DataModule TempModule;
  TempModule.Channel = 1;
  TempPacket.Modules.emplace_back(TempModule);
  TestProcessor(TempPacket);
  int MeanOfSamplesValue = 10;
  TestProcessor.setMeanOfSamples(MeanOfSamplesValue);
  EXPECT_EQ(TestProcessor.MeanOfNrOfSamples, MeanOfSamplesValue);
  EXPECT_EQ(TestProcessor.ProcessingInstances.at(TempModule.Channel).getMeanOfSamples(), MeanOfSamplesValue);
}

TEST(SampleProcessing, SetTimeStampLocation) {
  std::shared_ptr<ProducerBase> TestProducer(new ProducerStandIn());
  SampleProcessingStandIn TestProcessor(TestProducer);
  PacketData TempPacket;
  DataModule TempModule;
  TempModule.Channel = 1;
  TempPacket.Modules.emplace_back(TempModule);
  TestProcessor(TempPacket);
  TimeStampLocation UsedLocation = TimeStampLocation::End;
  TestProcessor.setTimeStampLocation(UsedLocation);
  EXPECT_EQ(TestProcessor.TSLocation, UsedLocation);
  EXPECT_EQ(TestProcessor.ProcessingInstances.at(TempModule.Channel).getTimeStampLocation(), UsedLocation);
}

DataModule getTestModule() {
  DataModule Module;
  Module.TimeStamp.Seconds = 42;
  Module.TimeStamp.SecondsFrac = 65;
  Module.Channel = 3;
  Module.Data.push_back(1);
  Module.Data.push_back(15);
  Module.Data.push_back(128);
  Module.Data.push_back(0);
  return Module;
}

TEST(SampleProcessing, ProcessCallTest) {
  std::shared_ptr<ProducerBase> TestProducer(new ProducerStandIn());
  SampleProcessingStandIn TestProcessor(TestProducer);
  REQUIRE_CALL(TestProcessor, serializeAndTransmitData(ANY(ProcessedSamples))).TIMES(1);
  PacketData TempPacket;
  TempPacket.Modules.emplace_back(getTestModule());
  TestProcessor(TempPacket);
}

TEST(SampleProcessing, ProcessFailCallTest) {
  std::shared_ptr<ProducerBase> TestProducer(new ProducerStandIn());
  SampleProcessingStandIn TestProcessor(TestProducer);
  ProcessedSamples CallTest;
  REQUIRE_CALL(TestProcessor, serializeAndTransmitData(ANY(ProcessedSamples))).TIMES(0);
  PacketData TempPacket;
  auto TempModule = getTestModule();
  TempModule.Data.clear();
  TempPacket.Modules.emplace_back(TempModule);
  TestProcessor(TempPacket);
  //EXPECT_EQ(CallTest.Samples.size(), TempModule.Data.size()); .LR_SIDE_EFFECT(CallTest = _1)
}

TEST(SampleProcessing, ProcessContentTest) {
  std::shared_ptr<ProducerBase> TestProducer(new ProducerStandIn());
  SampleProcessingStandIn TestProcessor(TestProducer);
  ProcessedSamples CallTest;
  REQUIRE_CALL(TestProcessor, serializeAndTransmitData(ANY(ProcessedSamples))).LR_SIDE_EFFECT(CallTest = _1).TIMES(1);
  PacketData TempPacket;
  auto TempModule = getTestModule();
  TempPacket.Modules.emplace_back(TempModule);
  TestProcessor(TempPacket);
  EXPECT_EQ(CallTest.Samples.size(), TempModule.Data.size());
  EXPECT_EQ(CallTest.TimeStamps.size(), TempModule.Data.size());
}

class ChannelProcessingTest : public ::testing::Test {
public:
  virtual void SetUp() override {
    Module = getTestModule();
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
