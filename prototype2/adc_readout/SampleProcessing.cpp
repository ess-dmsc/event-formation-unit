/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Sample processing of the ADC data.
 */

#include "SampleProcessing.h"

std::uint64_t CalcSampleTimeStamp(const RawTimeStamp &Start, const RawTimeStamp &End, const TimeStampLocation Location) {
  std::uint64_t StartNS = Start.GetTimeStampNS();
  std::uint64_t EndNS = End.GetTimeStampNS();
  if (TimeStampLocation::Start == Location) {
    return StartNS;
  } else if (TimeStampLocation::End == Location) {
    return EndNS;
  }
  return StartNS + (EndNS - StartNS) / 2;
}

ChannelProcessing::ChannelProcessing() {
  
}

ProcessedSamples ChannelProcessing::operator()(const DataModule &Samples) {
  ProcessedSamples ReturnSamples;
  for (size_t i = 0; i < Samples.Data.size(); i++) {
    if (0 == NrOfSamplesSummed) {
      TimeStampOfFirstSample = Samples.TimeStamp.GetOffsetTimeStamp(i * Samples.OversamplingFactor - (Samples.OversamplingFactor - 1));
    }
    SumOfSamples += Samples.Data.at(i);
    NrOfSamplesSummed++;
    if (NrOfSamplesSummed == MeanOfNrOfSamples) {
      int FinalOversamplingFactor = MeanOfNrOfSamples * Samples.OversamplingFactor;
      ReturnSamples.Samples.emplace_back(SumOfSamples / FinalOversamplingFactor);
      RawTimeStamp TimeStampOfLastSample = Samples.TimeStamp.GetOffsetTimeStamp(i * Samples.OversamplingFactor);
      ReturnSamples.TimeStamps.emplace_back(CalcSampleTimeStamp(TimeStampOfFirstSample, TimeStampOfLastSample, TSLocation));
      SumOfSamples = 0;
      NrOfSamplesSummed = 0;
    }
  }
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

SampleProcessing::SampleProcessing(std::shared_ptr<ProducerBase> Prod) : AdcDataProcessor(Prod) {
  
}

void SampleProcessing::setMeanOfSamples(int NrOfSamples) {
  MeanOfNrOfSamples = NrOfSamples;
  for (auto &Processor : ProcessingInstances) {
    Processor.second.setMeanOfSamples(NrOfSamples);
  }
}

void SampleProcessing::setTimeStampLocation(TimeStampLocation Location) {
  TSLocation = Location;
  for (auto &Processor : ProcessingInstances) {
    Processor.second.setTimeStampLocation(Location);
  }
}

void SampleProcessing::operator()(const PacketData &Data) {
  for (auto &Module : Data.Modules) {
    if (ProcessingInstances.find(Module.Channel) == ProcessingInstances.end()) {
      ProcessingInstances[Module.Channel] = ChannelProcessing();
      setMeanOfSamples(MeanOfNrOfSamples);
      setTimeStampLocation(TSLocation);
    }
    auto ResultingSamples = ProcessingInstances.at(Module.Channel)(Module);
    if (ResultingSamples.Samples.size() > 0) {
      serializeAndTransmitData(ResultingSamples);
    }
  }
}

void SampleProcessing::serializeAndTransmitData(ProcessedSamples const &Data) const {
  
}
