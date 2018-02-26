/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Sample processing of the ADC data.
 */

#include "SampleProcessing.h"

ChannelProcessing::ChannelProcessing() {
  
}

ProcessedSamples ChannelProcessing::operator()(const DataModule &Samples) {
//  int SampleCounter = 0;
  ProcessedSamples ReturnSamples;
  for (int i = 0; i < Samples.Data.size(); i++) {
    SumOfSamples += Samples.Data.at(i);
    NrOfSamplesSummed++;
    if (NrOfSamplesSummed == MeanOfNrOfSamples) {
      ReturnSamples.Samples.emplace_back(SumOfSamples / MeanOfNrOfSamples);
      SumOfSamples = 0;
      NrOfSamplesSummed = 0;
    }
  }
  return ReturnSamples;
}

void ChannelProcessing::setMeanOfSamples(unsigned int NrOfSamples) {
  MeanOfNrOfSamples = NrOfSamples;
}

void ChannelProcessing::reset() {
  SumOfSamples = 0;
  NrOfSamplesSummed = 0;
//  SampleBuffer.clear();
}

SampleProcessing::SampleProcessing(std::shared_ptr<Producer> Prod) : AdcDataProcessor(Prod) {
  
}

void SampleProcessing::setMeanOfSamples(unsigned int NrOfSamples) {
  
}


void SampleProcessing::operator()(const PacketData &Data) {
  
}
