/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests for the sample processing code (peak extraction and
 * individual samples).
 */

#include "../AdcReadoutConstants.h"
#include "../SampleProcessing.h"
#include "senv_data_generated.h"
#include <array>
#include <gtest/gtest.h>
#include <trompeloeil.hpp>

SamplingRun getTestModule() {
  SamplingRun Module;
  Module.TimeStamp.Seconds = 42;
  Module.TimeStamp.SecondsFrac = 65;
  Module.OversamplingFactor = 1;
  Module.Identifier.ChannelNr = 3;
  Module.Data.push_back(1);
  Module.Data.push_back(15);
  Module.Data.push_back(128);
  Module.Data.push_back(0);
  return Module;
}

using trompeloeil::_;

class SampleProcessingStandIn : public SampleProcessing {
public:
  SampleProcessingStandIn(std::shared_ptr<ProducerBase> Prod, std::string Name)
      : SampleProcessing(Prod, Name) {}
  using SampleProcessing::MeanOfNrOfSamples;
  using SampleProcessing::TSLocation;
  using SampleProcessing::ProcessingInstance;
  void serializeAndTransmitAlt(ProcessedSamples const &Data) {
    SampleProcessing::serializeAndTransmitData(Data);
  };
  MAKE_MOCK1(serializeAndTransmitData, void(const ProcessedSamples &),
             override);
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
class ProducerStandIn : public ProducerBase {
public:
  MAKE_MOCK2(produce, int(void *, size_t), override);
};
#pragma GCC diagnostic pop

TEST(SampleProcessing, NoSamples) {
  std::shared_ptr<ProducerStandIn> TestProducer(new ProducerStandIn());
  SampleProcessingStandIn TestProcessor(TestProducer, "SomeName");
  SamplingRun TempModule;
  TempModule.Identifier.ChannelNr = 1;
  TestProcessor.processData(TempModule);
  FORBID_CALL(*TestProducer.get(), produce(_, _));
}

TEST(SampleProcessing, SetMeanOfChannels) {
  std::shared_ptr<ProducerBase> TestProducer(new ProducerStandIn());
  SampleProcessingStandIn TestProcessor(TestProducer, "SomeName");
  int MeanOfSamplesValue = 10;
  TestProcessor.setMeanOfSamples(MeanOfSamplesValue);
  EXPECT_EQ(TestProcessor.MeanOfNrOfSamples, MeanOfSamplesValue);
  EXPECT_EQ(TestProcessor.ProcessingInstance.getMeanOfSamples(),
            MeanOfSamplesValue);
}

TEST(SampleProcessing, SetTimeStampLocation) {
  std::shared_ptr<ProducerBase> TestProducer(new ProducerStandIn());
  SampleProcessingStandIn TestProcessor(TestProducer, "SomeName");
  TimeStampLocation UsedLocation = TimeStampLocation::End;
  TestProcessor.setTimeStampLocation(UsedLocation);
  EXPECT_EQ(TestProcessor.TSLocation, UsedLocation);
  EXPECT_EQ(TestProcessor.ProcessingInstance.getTimeStampLocation(),
            UsedLocation);
}

TEST(SampleProcessing, ProcessCallTest) {
  std::shared_ptr<ProducerBase> TestProducer(new ProducerStandIn());
  SampleProcessingStandIn TestProcessor(TestProducer, "SomeName");
  REQUIRE_CALL(TestProcessor, serializeAndTransmitData(ANY(ProcessedSamples)))
      .TIMES(1);
  TestProcessor.processData(getTestModule());
}

TEST(SampleProcessing, ProcessFailCallTest) {
  std::shared_ptr<ProducerBase> TestProducer(new ProducerStandIn());
  SampleProcessingStandIn TestProcessor(TestProducer, "SomeName");
  ProcessedSamples CallTest;
  FORBID_CALL(TestProcessor, serializeAndTransmitData(_));
  auto TempModule = getTestModule();
  TempModule.Data.clear();
  TestProcessor.processData(TempModule);
}

TEST(SampleProcessing, ProcessContentTest) {
  std::shared_ptr<ProducerBase> TestProducer(new ProducerStandIn());
  SampleProcessingStandIn TestProcessor(TestProducer, "SomeName");
  ProcessedSamples CallTest;
  REQUIRE_CALL(TestProcessor, serializeAndTransmitData(_))
      .LR_SIDE_EFFECT(CallTest = _1)
      .TIMES(1);
  auto TempModule = getTestModule();
  TestProcessor.processData(TempModule);
  EXPECT_EQ(CallTest.Samples.size(), TempModule.Data.size());
}

TEST(SampleProcessing, SerialisationProduceCallTest) {
  std::shared_ptr<ProducerBase> TestProducer(new ProducerStandIn());
  SampleProcessingStandIn TestProcessor(TestProducer, "SomeName");
  ProcessedSamples CallTest;
  REQUIRE_CALL(TestProcessor, serializeAndTransmitData(_))
      .LR_SIDE_EFFECT(CallTest = _1)
      .TIMES(1)
      .LR_SIDE_EFFECT(TestProcessor.serializeAndTransmitAlt(_1));
  REQUIRE_CALL(*dynamic_cast<ProducerStandIn *>(TestProducer.get()),
               produce(ANY(void *), ANY(size_t)))
      .TIMES(1)
      .RETURN(0);
  auto TempModule = getTestModule();
  TestProcessor.processData(TempModule);
}

TEST(SampleProcessing, SerialisationFlatbufferTest1) {
  std::shared_ptr<ProducerBase> TestProducer(new ProducerStandIn());
  std::string Name = "SomeTestName";
  SampleProcessingStandIn TestProcessor(TestProducer, Name);
  TestProcessor.setTimeStampLocation(TimeStampLocation::End);
  ProcessedSamples CallTest;
  std::array<std::uint8_t, 4096> TempBuffer;
  int BytesCopied = 0;
  REQUIRE_CALL(TestProcessor, serializeAndTransmitData(ANY(ProcessedSamples)))
      .LR_SIDE_EFFECT(CallTest = _1)
      .TIMES(1)
      .LR_SIDE_EFFECT(TestProcessor.serializeAndTransmitAlt(_1));
  REQUIRE_CALL(*dynamic_cast<ProducerStandIn *>(TestProducer.get()),
               produce(ANY(void *), ANY(size_t)))
      .TIMES(1)
      .RETURN(0)
      .LR_SIDE_EFFECT(
          std::memcpy(reinterpret_cast<void *>(&TempBuffer[0]), _1, _2);
          BytesCopied = _2;);
  auto TempModule = getTestModule();
  TestProcessor.processData(TempModule);

  ASSERT_TRUE(BytesCopied != 0);
  auto Verifier = flatbuffers::Verifier(&TempBuffer[0], BytesCopied);
  ASSERT_TRUE(VerifySampleEnvironmentDataBuffer(Verifier));
  auto SampleData = GetSampleEnvironmentData(&TempBuffer[0]);
  auto ExpectedName = Name + "_Adc" +
                      std::to_string(TempModule.Identifier.SourceID) + "_Ch" +
                      std::to_string(TempModule.Identifier.ChannelNr);
  EXPECT_EQ(SampleData->Name()->str(), ExpectedName);
  EXPECT_EQ(SampleData->PacketTimestamp(),
            TempModule.TimeStamp.GetTimeStampNS());
  EXPECT_NEAR(SampleData->TimeDelta(),
              (1e9 * TempModule.OversamplingFactor) / AdcTimerCounterMax, 0.05);
  EXPECT_EQ(SampleData->TimestampLocation(), Location::End);
  EXPECT_EQ(SampleData->Channel(), TempModule.Identifier.ChannelNr);
  EXPECT_TRUE(SampleData->MessageCounter() == 0);
  EXPECT_EQ(SampleData->Values()->size(), TempModule.Data.size());
  EXPECT_EQ(flatbuffers::IsFieldPresent(SampleData,
                                        SampleEnvironmentData::VT_TIMESTAMPS),
            false);
}

TEST(SampleProcessing, SerialisationFlatbufferTest3) {
  std::shared_ptr<ProducerBase> TestProducer(new ProducerStandIn());
  std::string Name = "SomeTestName";
  SampleProcessingStandIn TestProcessor(TestProducer, Name);
  TestProcessor.setTimeStampLocation(TimeStampLocation::End);
  TestProcessor.setSerializeTimestamps(true);
  ProcessedSamples CallTest;
  std::array<std::uint8_t, 4096> TempBuffer;
  int BytesCopied = 0;
  REQUIRE_CALL(TestProcessor, serializeAndTransmitData(ANY(ProcessedSamples)))
      .LR_SIDE_EFFECT(CallTest = _1)
      .TIMES(1)
      .LR_SIDE_EFFECT(TestProcessor.serializeAndTransmitAlt(_1));
  REQUIRE_CALL(*dynamic_cast<ProducerStandIn *>(TestProducer.get()),
               produce(ANY(void *), ANY(size_t)))
      .TIMES(1)
      .RETURN(0)
      .LR_SIDE_EFFECT(
          std::memcpy(reinterpret_cast<void *>(&TempBuffer[0]), _1, _2);
          BytesCopied = _2;);
  auto TempModule = getTestModule();
  TestProcessor.processData(TempModule);

  ASSERT_TRUE(BytesCopied != 0);
  auto Verifier = flatbuffers::Verifier(&TempBuffer[0], BytesCopied);
  ASSERT_TRUE(VerifySampleEnvironmentDataBuffer(Verifier));
  auto SampleData = GetSampleEnvironmentData(&TempBuffer[0]);
  auto ExpectedName = Name + "_Adc" +
                      std::to_string(TempModule.Identifier.SourceID) + "_Ch" +
                      std::to_string(TempModule.Identifier.ChannelNr);
  EXPECT_EQ(SampleData->Name()->str(), ExpectedName);
  EXPECT_EQ(SampleData->PacketTimestamp(),
            TempModule.TimeStamp.GetTimeStampNS());
  EXPECT_NEAR(SampleData->TimeDelta(),
              (1e9 * TempModule.OversamplingFactor) / AdcTimerCounterMax, 0.05);
  EXPECT_EQ(SampleData->TimestampLocation(), Location::End);
  EXPECT_EQ(SampleData->Channel(), TempModule.Identifier.ChannelNr);
  EXPECT_TRUE(SampleData->MessageCounter() == 0);
  EXPECT_EQ(SampleData->Values()->size(), TempModule.Data.size());
  ASSERT_EQ(flatbuffers::IsFieldPresent(SampleData,
                                        SampleEnvironmentData::VT_TIMESTAMPS),
            true);
  EXPECT_EQ(SampleData->Timestamps()->size(), TempModule.Data.size());
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
TEST(SampleProcessing, SerialisationFlatbufferTest2) {
  std::shared_ptr<ProducerBase> TestProducer(new ProducerStandIn());
  std::string Name = "SomeTestName";
  SampleProcessingStandIn TestProcessor(TestProducer, Name);
  TestProcessor.setTimeStampLocation(TimeStampLocation::End);
  unsigned int MessageCounter = 0;
  auto TestMessage = [&MessageCounter](void *DataPtr, int Bytes) {
    auto SampleData = GetSampleEnvironmentData(DataPtr);
    EXPECT_EQ(SampleData->MessageCounter(), MessageCounter);
    MessageCounter++;
  };
  REQUIRE_CALL(TestProcessor, serializeAndTransmitData(ANY(ProcessedSamples)))
      .TIMES(2)
      .LR_SIDE_EFFECT(TestProcessor.serializeAndTransmitAlt(_1));
  REQUIRE_CALL(*dynamic_cast<ProducerStandIn *>(TestProducer.get()),
               produce(ANY(void *), ANY(size_t)))
      .TIMES(2)
      .RETURN(0)
      .LR_SIDE_EFFECT(TestMessage(_1, _2));
  TestProcessor.processData(getTestModule());
  TestProcessor.processData(getTestModule());
}
#pragma GCC diagnostic pop

class ChannelProcessingTest : public ::testing::Test {
public:
  virtual void SetUp() override { Module = getTestModule(); }
  virtual void TearDown() override { Module.Data.clear(); }
  SamplingRun Module;
};

TEST_F(ChannelProcessingTest, NumberOfSamplesSummed1) {
  ChannelProcessing Processing;
  Module.OversamplingFactor = 2;
  Processing.setMeanOfSamples(1);
  Processing.setTimeStampLocation(TimeStampLocation::Start);
  auto Result = Processing.processModule(Module);
  EXPECT_EQ(Result.Samples.size(), Module.Data.size());
}

TEST_F(ChannelProcessingTest, NumberOfSamplesSummed2) {
  ChannelProcessing Processing;
  Module.OversamplingFactor = 1;
  Processing.setMeanOfSamples(1);
  Processing.setTimeStampLocation(TimeStampLocation::Start);
  auto Result = Processing.processModule(Module);
  EXPECT_EQ(Result.Samples.size(), Module.Data.size());
}

TEST_F(ChannelProcessingTest, NumberOfSamplesSummed3) {
  ChannelProcessing Processing;
  Module.OversamplingFactor = 1;
  Processing.setMeanOfSamples(2);
  Processing.setTimeStampLocation(TimeStampLocation::Start);
  auto Result = Processing.processModule(Module);
  EXPECT_EQ(Result.Samples.size(), Module.Data.size() / 2);
}

TEST_F(ChannelProcessingTest, NumberOfSamplesSummed4) {
  ChannelProcessing Processing;
  Module.OversamplingFactor = 2;
  Processing.setMeanOfSamples(2);
  Processing.setTimeStampLocation(TimeStampLocation::Start);
  auto Result = Processing.processModule(Module);
  EXPECT_EQ(Result.Samples.size(), Module.Data.size() / 2);
}

TEST_F(ChannelProcessingTest, OversamplingAndTime1) {
  ChannelProcessing Processing;
  Processing.setMeanOfSamples(1);
  Processing.setTimeStampLocation(TimeStampLocation::Start);
  auto Result = Processing.processModule(Module);
  for (unsigned int i = 0; i < Result.TimeStamps.size(); i++) {
    EXPECT_EQ(Module.TimeStamp.GetOffsetTimeStamp(i).GetTimeStampNS(),
              Result.TimeStamps.at(i));
  }
}

TEST_F(ChannelProcessingTest, OversamplingAndTime2) {
  ChannelProcessing Processing;
  Processing.setMeanOfSamples(2);
  Processing.setTimeStampLocation(TimeStampLocation::Start);
  auto Result = Processing.processModule(Module);
  for (unsigned int i = 0; i < Result.TimeStamps.size(); i++) {
    EXPECT_EQ(Module.TimeStamp.GetOffsetTimeStamp(i * 2).GetTimeStampNS(),
              Result.TimeStamps.at(i));
  }
}

TEST_F(ChannelProcessingTest, OversamplingAndTime3) {
  ChannelProcessing Processing;
  Processing.setMeanOfSamples(2);
  Processing.setTimeStampLocation(TimeStampLocation::End);
  auto Result = Processing.processModule(Module);
  for (unsigned int i = 0; i < Result.TimeStamps.size(); i++) {
    EXPECT_EQ(Module.TimeStamp.GetOffsetTimeStamp(i * 2 + 1).GetTimeStampNS(),
              Result.TimeStamps.at(i));
  }
}

TEST_F(ChannelProcessingTest, OversamplingAndTime4) {
  ChannelProcessing Processing;
  Module.OversamplingFactor = 4;
  Processing.setMeanOfSamples(1);
  Processing.setTimeStampLocation(TimeStampLocation::Start);
  auto Result = Processing.processModule(Module);
  for (unsigned int i = 0; i < Result.TimeStamps.size(); i++) {
    EXPECT_EQ(Module.TimeStamp.GetOffsetTimeStamp(i * 4 - 3).GetTimeStampNS(),
              Result.TimeStamps.at(i));
  }
}

TEST_F(ChannelProcessingTest, OversamplingAndTime5) {
  ChannelProcessing Processing;
  Module.OversamplingFactor = 4;
  Processing.setMeanOfSamples(1);
  Processing.setTimeStampLocation(TimeStampLocation::End);
  auto Result = Processing.processModule(Module);
  for (unsigned int i = 0; i < Result.TimeStamps.size(); i++) {
    EXPECT_EQ(Module.TimeStamp.GetOffsetTimeStamp(i * 4).GetTimeStampNS(),
              Result.TimeStamps.at(i));
  }
}

TEST_F(ChannelProcessingTest, OversamplingAndTime6) {
  ChannelProcessing Processing;
  Module.OversamplingFactor = 4;
  Processing.setMeanOfSamples(2);
  Processing.setTimeStampLocation(TimeStampLocation::End);
  auto Result = Processing.processModule(Module);
  for (unsigned int i = 0; i < Result.TimeStamps.size(); i++) {
    EXPECT_EQ(
        Module.TimeStamp.GetOffsetTimeStamp(i * 2 * 4 + 4).GetTimeStampNS(),
        Result.TimeStamps.at(i));
  }
}

TEST_F(ChannelProcessingTest, OversamplingAndTime7) {
  ChannelProcessing Processing;
  Module.OversamplingFactor = 4;
  Processing.setMeanOfSamples(2);
  Processing.setTimeStampLocation(TimeStampLocation::Start);
  auto Result = Processing.processModule(Module);
  for (unsigned int i = 0; i < Result.TimeStamps.size(); i++) {
    EXPECT_EQ(
        Module.TimeStamp.GetOffsetTimeStamp(i * 2 * 4 - 3).GetTimeStampNS(),
        Result.TimeStamps.at(i));
  }
}

TEST_F(ChannelProcessingTest, OversamplingAndTime8) {
  ChannelProcessing Processing;
  Module.OversamplingFactor = 4;
  Processing.setMeanOfSamples(2);
  Processing.setTimeStampLocation(TimeStampLocation::Middle);
  auto Result = Processing.processModule(Module);
  for (unsigned int i = 0; i < Result.TimeStamps.size(); i++) {
    std::uint64_t StartTS =
        Module.TimeStamp.GetOffsetTimeStamp(i * 2 * 4 - 3).GetTimeStampNS();
    std::uint64_t EndTS =
        Module.TimeStamp.GetOffsetTimeStamp(i * 2 * 4 + 4).GetTimeStampNS();
    EXPECT_EQ(StartTS + (EndTS - StartTS) / 2, Result.TimeStamps.at(i));
  }
}

TEST_F(ChannelProcessingTest, OversamplingAndTime9) {
  ChannelProcessing Processing;
  Module.OversamplingFactor = 1;
  Processing.setMeanOfSamples(1);
  Processing.setTimeStampLocation(TimeStampLocation::Middle);
  auto Result = Processing.processModule(Module);
  for (unsigned int i = 0; i < Result.TimeStamps.size(); i++) {
    EXPECT_EQ(Module.TimeStamp.GetOffsetTimeStamp(i).GetTimeStampNS(),
              Result.TimeStamps.at(i));
  }
}

TEST_F(ChannelProcessingTest, OversamplingAndTime10) {
  ChannelProcessing Processing;
  Module.OversamplingFactor = 1;
  Processing.setMeanOfSamples(1);
  Processing.setTimeStampLocation(TimeStampLocation::End);
  auto Result = Processing.processModule(Module);
  for (unsigned int i = 0; i < Result.TimeStamps.size(); i++) {
    EXPECT_EQ(Module.TimeStamp.GetOffsetTimeStamp(i).GetTimeStampNS(),
              Result.TimeStamps.at(i));
  }
}

TEST_F(ChannelProcessingTest, DefaultSetup) {
  ChannelProcessing Processing;
  auto Result = Processing.processModule(Module);
  EXPECT_EQ(Module.Data, Result.Samples);
}

TEST_F(ChannelProcessingTest, Oversampling2X) {
  ChannelProcessing Processing;
  Processing.setMeanOfSamples(2);
  std::vector<std::uint16_t> ExpectedResult{8, 64};
  auto Result = Processing.processModule(Module);
  EXPECT_EQ(ExpectedResult, Result.Samples);
}

TEST_F(ChannelProcessingTest, Oversampling3X_OneModule) {
  ChannelProcessing Processing;
  Processing.setMeanOfSamples(3);
  std::vector<std::uint16_t> ExpectedResult{
      48,
  };
  auto Result = Processing.processModule(Module);
  EXPECT_EQ(ExpectedResult, Result.Samples);
}

TEST_F(ChannelProcessingTest, Oversampling3X_TwoModules) {
  ChannelProcessing Processing;
  Processing.setMeanOfSamples(3);
  Processing.processModule(Module);
  auto Result = Processing.processModule(Module);
  std::vector<std::uint16_t> ExpectedResult{
      5,
  };
  EXPECT_EQ(ExpectedResult, Result.Samples);
}

TEST(CalcTimeStamp, StartTest) {
  RawTimeStamp TS1{53, 500};
  RawTimeStamp TS2{53, 1000};
  EXPECT_EQ(CalcSampleTimeStamp(TS1, TS2, TimeStampLocation::Start),
            TS1.GetTimeStampNS());
}

TEST(CalcTimeStamp, EndTest) {
  RawTimeStamp TS1{53, 500};
  RawTimeStamp TS2{53, 1000};
  EXPECT_EQ(CalcSampleTimeStamp(TS1, TS2, TimeStampLocation::End),
            TS2.GetTimeStampNS());
}

TEST(CalcTimeStamp, MiddleTest1) {
  RawTimeStamp TS1{53, 500};
  RawTimeStamp TSMid{53, 750};
  RawTimeStamp TS2{53, 1000};
  EXPECT_EQ(CalcSampleTimeStamp(TS1, TS2, TimeStampLocation::Middle),
            TSMid.GetTimeStampNS());
}

TEST(CalcTimeStamp, MiddleTest2) {
  RawTimeStamp TS{53, 0};
  RawTimeStamp TSMid{53, 0};
  EXPECT_EQ(CalcSampleTimeStamp(TS.GetOffsetTimeStamp(-150),
                                TS.GetOffsetTimeStamp(150),
                                TimeStampLocation::Middle),
            TSMid.GetTimeStampNS());
}

TEST(CalcTimeStamp, MiddleTest3) {
  RawTimeStamp TS{53, 1};
  RawTimeStamp TSMid{53, 1};
  EXPECT_EQ(CalcSampleTimeStamp(TS.GetOffsetTimeStamp(-150),
                                TS.GetOffsetTimeStamp(150),
                                TimeStampLocation::Middle),
            TSMid.GetTimeStampNS());
}

TEST(CalcTimeStamp, MiddleTest4) {
  RawTimeStamp TS{53, AdcTimerCounterMax - 5};
  RawTimeStamp TSMid{53, AdcTimerCounterMax - 5};
  EXPECT_EQ(CalcSampleTimeStamp(TS.GetOffsetTimeStamp(-150),
                                TS.GetOffsetTimeStamp(150),
                                TimeStampLocation::Middle),
            TSMid.GetTimeStampNS());
}
