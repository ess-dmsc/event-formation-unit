/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Sample processing of the ADC data.
 */

#include "SampleProcessing.h"
#include "AdcReadoutConstants.h"
#include "senv_data_generated.h"
#include <cmath>
#include <algorithm>

std::uint64_t CalcSampleTimeStamp(const TimeStamp &Start, const TimeStamp &End,
                                  const TimeStampLocation Location) {
  std::uint64_t StartNS = Start.getTimeStampNS();
  std::uint64_t EndNS = End.getTimeStampNS();
  if (TimeStampLocation::Start == Location) {
    return StartNS;
  }
  if (TimeStampLocation::End == Location) {
    return EndNS;
  }
  return StartNS + (EndNS - StartNS) / 2;
}

ProcessedSamples ChannelProcessing::processModule(const SamplingRun &Samples) {
  int FinalOversamplingFactor = MeanOfNrOfSamples * Samples.OversamplingFactor;
  if (FinalOversamplingFactor == 0) {
    FinalOversamplingFactor = 1;
  }
  size_t SampleIndex{0};
  size_t TotalNumberOfSamples =
      (Samples.Data.size() + NrOfSamplesSummed) / MeanOfNrOfSamples;
  ProcessedSamples ReturnSamples(TotalNumberOfSamples);

  ReturnSamples.TimeDelta = FinalOversamplingFactor *
                            Samples.ReferenceTimestamp.getClockCycleLength();
  std::uint64_t TimeStampOffset{0};
  if (TSLocation == TimeStampLocation::Middle) {
    TimeStampOffset =
        std::llround(0.5 * (ReturnSamples.TimeDelta / FinalOversamplingFactor) *
                     (FinalOversamplingFactor - 1));
  } else if (TSLocation == TimeStampLocation::End) {
    TimeStampOffset =
        std::llround((ReturnSamples.TimeDelta / FinalOversamplingFactor) *
                     (FinalOversamplingFactor - 1));
  }

  for (size_t i = 0; i < Samples.Data.size(); i++) {
    if (0 == NrOfSamplesSummed) {
      TimeStampOfFirstSample = Samples.StartTime.getOffsetTimeStamp(
          i * Samples.OversamplingFactor - (Samples.OversamplingFactor - 1));
    }
    SumOfSamples += Samples.Data[i];
    NrOfSamplesSummed++;
    if (NrOfSamplesSummed == MeanOfNrOfSamples) {
      ReturnSamples.Samples[SampleIndex] =
          SumOfSamples / FinalOversamplingFactor;
      ReturnSamples.TimeStamps[SampleIndex] =
          TimeStampOfFirstSample.getTimeStampNS() + TimeStampOffset;

      ChannelProcessing::reset();
      ++SampleIndex;
    }
  }
  if (not ReturnSamples.TimeStamps.empty()) {
    ReturnSamples.TimeStamp = ReturnSamples.TimeStamps[0];
  }
  ReturnSamples.Identifier = Samples.Identifier;
  return ReturnSamples;
}

void ChannelProcessing::setMeanOfSamples(int NrOfSamples) {
  MeanOfNrOfSamples = NrOfSamples;
  reset();
}

void ChannelProcessing::setTimeStampLocation(TimeStampLocation Location) {
  TSLocation = Location;
}

void ChannelProcessing::reset() {
  SumOfSamples = 0;
  NrOfSamplesSummed = 0;
}

SampleProcessing::SampleProcessing(std::shared_ptr<ProducerBase> Prod,
                                   std::string Name, OffsetTime UsedOffset)
    : AdcDataProcessor(std::move(Prod)), AdcName(std::move(Name)), TimeOffset(UsedOffset) {}

void SampleProcessing::setMeanOfSamples(int NrOfSamples) {
  MeanOfNrOfSamples = NrOfSamples;
  ProcessingInstance.setMeanOfSamples(MeanOfNrOfSamples);
}

void SampleProcessing::setSerializeTimestamps(bool SerializeTimeStamps) {
  SampleTimestamps = SerializeTimeStamps;
}

void SampleProcessing::setTimeStampLocation(TimeStampLocation Location) {
  TSLocation = Location;
  ProcessingInstance.setTimeStampLocation(TSLocation);
}

void SampleProcessing::processData(const SamplingRun &Data) {
  auto ResultingSamples = ProcessingInstance.processModule(Data);
  if (not ResultingSamples.Samples.empty()) {
    serializeAndTransmitData(ResultingSamples);
  }
}

void SampleProcessing::serializeAndTransmitData(ProcessedSamples const &Data) {
  flatbuffers::FlatBufferBuilder builder;
  auto FBSampleData = builder.CreateVector(Data.Samples);
  flatbuffers::Offset<flatbuffers::Vector<std::uint64_t>> FBTimeStamps;
  if (SampleTimestamps) {
    auto SampleTimes = Data.TimeStamps;
    std::transform(SampleTimes.begin(), SampleTimes.end(), SampleTimes.begin(), [this](auto const &TS){
      return TimeOffset.calcTimestampNS(TS);
    });
    FBTimeStamps = builder.CreateVector(SampleTimes);
  }

  auto FBName = builder.CreateString(
      AdcName + "_Adc" + std::to_string(Data.Identifier.SourceID) + "_Ch" +
      std::to_string(Data.Identifier.ChannelNr) + "_waveform");
  SampleEnvironmentDataBuilder MessageBuilder(builder);
  MessageBuilder.add_Name(FBName);
  MessageBuilder.add_Values(FBSampleData);
  if (SampleTimestamps) {
    MessageBuilder.add_Timestamps(FBTimeStamps);
  }
  MessageBuilder.add_Channel(Data.Identifier.ChannelNr);
  auto NewOffsetTimestamp = TimeOffset.calcTimestampNS(Data.TimeStamp);
  MessageBuilder.add_PacketTimestamp(NewOffsetTimestamp);
  MessageBuilder.add_TimeDelta(Data.TimeDelta);

  MessageBuilder.add_MessageCounter(MessageCounter++);
  MessageBuilder.add_TimestampLocation(
      Location(TimeLocSerialisationMap.at(TSLocation)));
  builder.Finish(MessageBuilder.Finish(), SampleEnvironmentDataIdentifier());
  ProducerPtr->produce({builder.GetBufferPointer(), builder.GetSize()},
                       NewOffsetTimestamp / 1000000);
}
