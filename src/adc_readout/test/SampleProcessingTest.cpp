/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests for the sample processing code (peak extraction and
 * individual samples).
 */

#include "../SampleProcessing.h"
#include "../AdcReadoutConstants.h"
#include "senv_data_generated.h"
#include <array>
#include <cstring>
#include <gtest/gtest.h>
#include <trompeloeil.hpp>

SamplingRun getTestModule() {
  SamplingRun Module;
  Module.StartTime = {{42, 65}, TimeStamp::ClockMode::External};
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
      : SampleProcessing(std::move(Prod), std::move(Name)) {}
  using SampleProcessing::MeanOfNrOfSamples;
  using SampleProcessing::ProcessingInstance;
  using SampleProcessing::TSLocation;
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
  MAKE_MOCK2(produce, int(nonstd::span<const std::uint8_t>, std::int64_t),
             override);
};
#pragma GCC diagnostic pop

TEST(SampleProcessing, NoSamples) {
  auto TestProducer = std::make_shared<ProducerStandIn>();
  SampleProcessingStandIn TestProcessor(TestProducer, "SomeName");
  SamplingRun TempModule;
  TempModule.Identifier.ChannelNr = 1;
  TestProcessor.processData(TempModule);
  FORBID_CALL(*TestProducer, produce(_, _));
}

TEST(SampleProcessing, SetMeanOfChannels) {
  auto TestProducer = std::make_shared<ProducerStandIn>();
  SampleProcessingStandIn TestProcessor(TestProducer, "SomeName");
  int MeanOfSamplesValue = 10;
  TestProcessor.setMeanOfSamples(MeanOfSamplesValue);
  EXPECT_EQ(TestProcessor.MeanOfNrOfSamples, MeanOfSamplesValue);
  EXPECT_EQ(TestProcessor.ProcessingInstance.getMeanOfSamples(),
            MeanOfSamplesValue);
}

TEST(SampleProcessing, SetTimeStampLocation) {
  auto TestProducer = std::make_shared<ProducerStandIn>();
  SampleProcessingStandIn TestProcessor(TestProducer, "SomeName");
  TimeStampLocation UsedLocation = TimeStampLocation::End;
  TestProcessor.setTimeStampLocation(UsedLocation);
  EXPECT_EQ(TestProcessor.TSLocation, UsedLocation);
  EXPECT_EQ(TestProcessor.ProcessingInstance.getTimeStampLocation(),
            UsedLocation);
}

TEST(SampleProcessing, ProcessCallTest) {
  auto TestProducer = std::make_shared<ProducerStandIn>();
  SampleProcessingStandIn TestProcessor(TestProducer, "SomeName");
  REQUIRE_CALL(TestProcessor, serializeAndTransmitData(ANY(ProcessedSamples)))
      .TIMES(1);
  TestProcessor.processData(getTestModule());
}

TEST(SampleProcessing, ProcessFailCallTest) {
  auto TestProducer = std::make_shared<ProducerStandIn>();
  SampleProcessingStandIn TestProcessor(TestProducer, "SomeName");
  ProcessedSamples CallTest;
  FORBID_CALL(TestProcessor, serializeAndTransmitData(_));
  auto TempModule = getTestModule();
  TempModule.Data.clear();
  TestProcessor.processData(TempModule);
}

TEST(SampleProcessing, ProcessContentTest) {
  auto TestProducer = std::make_shared<ProducerStandIn>();
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
  auto TestProducer = std::make_shared<ProducerStandIn>();
  SampleProcessingStandIn TestProcessor(TestProducer, "SomeName");
  ProcessedSamples CallTest;
  REQUIRE_CALL(TestProcessor, serializeAndTransmitData(_))
      .LR_SIDE_EFFECT(CallTest = _1)
      .TIMES(1)
      .LR_SIDE_EFFECT(TestProcessor.serializeAndTransmitAlt(_1));
  REQUIRE_CALL(*dynamic_cast<ProducerStandIn *>(TestProducer.get()),
               produce(_, _))
      .TIMES(1)
      .RETURN(0);
  auto TempModule = getTestModule();
  TestProcessor.processData(TempModule);
}

TEST(SampleProcessing, SerialisationFlatbufferTest1) {
  auto TestProducer = std::make_shared<ProducerStandIn>();
  std::string Name = "SomeTestName";
  SampleProcessingStandIn TestProcessor(TestProducer, Name);
  TestProcessor.setTimeStampLocation(TimeStampLocation::End);
  ProcessedSamples CallTest;
  std::array<std::uint8_t, 4096> TempBuffer{};
  int BytesCopied = 0;
  REQUIRE_CALL(TestProcessor, serializeAndTransmitData(ANY(ProcessedSamples)))
      .LR_SIDE_EFFECT(CallTest = _1)
      .TIMES(1)
      .LR_SIDE_EFFECT(TestProcessor.serializeAndTransmitAlt(_1));
  REQUIRE_CALL(*dynamic_cast<ProducerStandIn *>(TestProducer.get()),
               produce(_, _))
      .TIMES(1)
      .RETURN(0)
      .LR_SIDE_EFFECT(std::memcpy(reinterpret_cast<void *>(&TempBuffer[0]),
                                  reinterpret_cast<const void *>(_1.data()),
                                  _1.size_bytes());
                      BytesCopied = _1.size_bytes(););
  auto TempModule = getTestModule();
  TestProcessor.processData(TempModule);

  ASSERT_TRUE(BytesCopied != 0);
  auto Verifier = flatbuffers::Verifier(&TempBuffer[0], BytesCopied);
  ASSERT_TRUE(VerifySampleEnvironmentDataBuffer(Verifier));
  auto SampleData = GetSampleEnvironmentData(&TempBuffer[0]);
  auto ExpectedName =
      Name + "_Adc" + std::to_string(TempModule.Identifier.SourceID) + "_Ch" +
      std::to_string(TempModule.Identifier.ChannelNr) + "_waveform";
  EXPECT_EQ(SampleData->Name()->str(), ExpectedName);
  EXPECT_EQ(SampleData->PacketTimestamp(),
            TempModule.StartTime.getTimeStampNS());
  EXPECT_NEAR(SampleData->TimeDelta(),
              (1e9 * TempModule.OversamplingFactor) /
                  (TimerClockFrequencyExternal / 2),
              0.05);
  EXPECT_EQ(SampleData->TimestampLocation(), Location::End);
  EXPECT_EQ(SampleData->Channel(), TempModule.Identifier.ChannelNr);
  EXPECT_TRUE(SampleData->MessageCounter() == 0);
  EXPECT_EQ(SampleData->Values()->size(), TempModule.Data.size());
  EXPECT_EQ(flatbuffers::IsFieldPresent(SampleData,
                                        SampleEnvironmentData::VT_TIMESTAMPS),
            false);
}

TEST(SampleProcessing, SerialisationFlatbufferTest3) {
  auto TestProducer = std::make_shared<ProducerStandIn>();
  std::string Name = "SomeTestName";
  SampleProcessingStandIn TestProcessor(TestProducer, Name);
  TestProcessor.setTimeStampLocation(TimeStampLocation::End);
  TestProcessor.setSerializeTimestamps(true);
  ProcessedSamples CallTest;
  std::array<std::uint8_t, 4096> TempBuffer{};
  int BytesCopied = 0;
  REQUIRE_CALL(TestProcessor, serializeAndTransmitData(ANY(ProcessedSamples)))
      .LR_SIDE_EFFECT(CallTest = _1)
      .TIMES(1)
      .LR_SIDE_EFFECT(TestProcessor.serializeAndTransmitAlt(_1));
  REQUIRE_CALL(*dynamic_cast<ProducerStandIn *>(TestProducer.get()),
               produce(_, _))
      .TIMES(1)
      .RETURN(0)
      .LR_SIDE_EFFECT(std::memcpy(reinterpret_cast<void *>(&TempBuffer[0]),
                                  reinterpret_cast<const void *>(_1.data()),
                                  _1.size_bytes());
                      BytesCopied = _1.size_bytes(););
  auto TempModule = getTestModule();
  TestProcessor.processData(TempModule);

  ASSERT_TRUE(BytesCopied != 0);
  auto Verifier = flatbuffers::Verifier(&TempBuffer[0], BytesCopied);
  ASSERT_TRUE(VerifySampleEnvironmentDataBuffer(Verifier));
  auto SampleData = GetSampleEnvironmentData(&TempBuffer[0]);
  auto ExpectedName =
      Name + "_Adc" + std::to_string(TempModule.Identifier.SourceID) + "_Ch" +
      std::to_string(TempModule.Identifier.ChannelNr) + "_waveform";
  EXPECT_EQ(SampleData->Name()->str(), ExpectedName);
  EXPECT_EQ(SampleData->PacketTimestamp(),
            TempModule.StartTime.getTimeStampNS());
  EXPECT_NEAR(SampleData->TimeDelta(),
              (1e9 * TempModule.OversamplingFactor) /
                  (TimerClockFrequencyExternal / 2),
              0.05);
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
  auto TestProducer = std::make_shared<ProducerStandIn>();
  std::string Name = "SomeTestName";
  SampleProcessingStandIn TestProcessor(TestProducer, Name);
  TestProcessor.setTimeStampLocation(TimeStampLocation::End);
  unsigned int MessageCounter = 0;
  auto TestMessage = [&MessageCounter](auto Data) {
    auto SampleData =
        GetSampleEnvironmentData(reinterpret_cast<const void *>(Data.data()));
    EXPECT_EQ(SampleData->MessageCounter(), MessageCounter);
    MessageCounter++;
  };
  REQUIRE_CALL(TestProcessor, serializeAndTransmitData(ANY(ProcessedSamples)))
      .TIMES(2)
      .LR_SIDE_EFFECT(TestProcessor.serializeAndTransmitAlt(_1));
  REQUIRE_CALL(*dynamic_cast<ProducerStandIn *>(TestProducer.get()),
               produce(_, _))
      .TIMES(2)
      .RETURN(0)
      .LR_SIDE_EFFECT(TestMessage(_1));
  TestProcessor.processData(getTestModule());
  TestProcessor.processData(getTestModule());
}
#pragma GCC diagnostic pop

class ChannelProcessingTest : public ::testing::Test {
public:
  void SetUp() override { Module = getTestModule(); }
  void TearDown() override { Module.Data.clear(); }
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
    EXPECT_EQ(Module.StartTime.getOffsetTimeStamp(i).getTimeStampNS(),
              Result.TimeStamps.at(i));
  }
}

TEST_F(ChannelProcessingTest, OversamplingAndTime2) {
  ChannelProcessing Processing;
  Processing.setMeanOfSamples(2);
  Processing.setTimeStampLocation(TimeStampLocation::Start);
  auto Result = Processing.processModule(Module);
  for (unsigned int i = 0; i < Result.TimeStamps.size(); i++) {
    EXPECT_EQ(Module.StartTime.getOffsetTimeStamp(i * 2).getTimeStampNS(),
              Result.TimeStamps.at(i));
  }
}

TEST_F(ChannelProcessingTest, OversamplingAndTime3) {
  ChannelProcessing Processing;
  Processing.setMeanOfSamples(2);
  Processing.setTimeStampLocation(TimeStampLocation::End);
  auto Result = Processing.processModule(Module);
  for (unsigned int i = 0; i < Result.TimeStamps.size(); i++) {
    EXPECT_EQ(Module.StartTime.getOffsetTimeStamp(i * 2 + 1).getTimeStampNS(),
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
    EXPECT_EQ(Module.StartTime.getOffsetTimeStamp(i * 4 - 3).getTimeStampNS(),
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
    EXPECT_EQ(Module.StartTime.getOffsetTimeStamp(i * 4).getTimeStampNS(),
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
        Module.StartTime.getOffsetTimeStamp(i * 2 * 4 + 4).getTimeStampNS(),
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
        Module.StartTime.getOffsetTimeStamp(i * 2 * 4 - 3).getTimeStampNS(),
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
        Module.StartTime.getOffsetTimeStamp(i * 2 * 4 - 3).getTimeStampNS();
    std::uint64_t EndTS =
        Module.StartTime.getOffsetTimeStamp(i * 2 * 4 + 4).getTimeStampNS();
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
    EXPECT_EQ(Module.StartTime.getOffsetTimeStamp(i).getTimeStampNS(),
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
    EXPECT_EQ(Module.StartTime.getOffsetTimeStamp(i).getTimeStampNS(),
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

static auto const ExtClk = TimeStamp::ClockMode::External;

TEST(CalcTimeStamp, StartTest) {
  TimeStamp TS1{{53, 500}, ExtClk};
  TimeStamp TS2{{53, 1000}, ExtClk};
  EXPECT_EQ(CalcSampleTimeStamp(TS1, TS2, TimeStampLocation::Start),
            TS1.getTimeStampNS());
}

TEST(CalcTimeStamp, EndTest) {
  TimeStamp TS1{{53, 500}, ExtClk};
  TimeStamp TS2{{53, 1000}, ExtClk};
  EXPECT_EQ(CalcSampleTimeStamp(TS1, TS2, TimeStampLocation::End),
            TS2.getTimeStampNS());
}

TEST(CalcTimeStamp, MiddleTest1) {
  TimeStamp TS1{{53, 500}, ExtClk};
  TimeStamp TSMid{{53, 750}, ExtClk};
  TimeStamp TS2{{53, 1000}, ExtClk};
  EXPECT_EQ(CalcSampleTimeStamp(TS1, TS2, TimeStampLocation::Middle),
            TSMid.getTimeStampNS());
}

TEST(CalcTimeStamp, MiddleTest2) {
  TimeStamp TS{{53, 0}, ExtClk};
  TimeStamp TSMid{{53, 0}, ExtClk};
  EXPECT_EQ(CalcSampleTimeStamp(TS.getOffsetTimeStamp(-150),
                                TS.getOffsetTimeStamp(150),
                                TimeStampLocation::Middle),
            TSMid.getTimeStampNS());
}

TEST(CalcTimeStamp, MiddleTest3) {
  TimeStamp TS{{53, 1}, ExtClk};
  TimeStamp TSMid{{53, 1}, ExtClk};
  EXPECT_EQ(CalcSampleTimeStamp(TS.getOffsetTimeStamp(-150),
                                TS.getOffsetTimeStamp(150),
                                TimeStampLocation::Middle),
            TSMid.getTimeStampNS());
}

TEST(CalcTimeStamp, MiddleTest4) {
  TimeStamp TS{{53, TimerClockFrequencyExternal / 2 - 5}, ExtClk};
  TimeStamp TSMid{{53, TimerClockFrequencyExternal / 2 - 5}, ExtClk};
  EXPECT_EQ(CalcSampleTimeStamp(TS.getOffsetTimeStamp(-150),
                                TS.getOffsetTimeStamp(150),
                                TimeStampLocation::Middle),
            TSMid.getTimeStampNS());
}
