//
//  ADC_Readout.cpp
//  ADC_Data_Receiver
//
//  Created by Jonas Nilsson on 2017-10-17.
//  Copyright © 2017 European Spallation Source. All rights reserved.
//

#include "PeakFinder.h"
#include <cmath>
#include <limits>
#include "MonitorPeakData_generated.h"

PeakFinder::PeakFinder(std::shared_ptr<Producer> Prod, bool PositivePolarity) : AdcDataProcessor(Prod), PositivePulse(PositivePolarity) {
  
}

void PeakFinder::operator()(const PacketData &Data) {
  if (Data.Type != PacketType::Data) {
    return;
  }
  for (auto &Module : Data.Modules) {
    auto Result = FindPeak(Module.Data);
    std::uint16_t Amplitude;
    std::uint16_t SampleNr;
    if (PositivePulse) {
      Amplitude = Result.Max;
      SampleNr = Result.MaxLocation;
    } else {
      Amplitude = Result.Min;
      SampleNr = Result.MinLocation;
    }
    std::uint64_t PeakTimeStamp = TimeStamp::CalcSample(Module.TimeStampSeconds, Module.TimeStampSecondsFrac, SampleNr);
    SendData(PeakTimeStamp, Amplitude, Module.Channel);
    
  }
}

void PeakFinder::SendData(const std::uint64_t &TimeStamp, const std::uint16_t &Amplitude, const std::uint16_t &Channel) {
  flatbuffers::FlatBufferBuilder builder;
  auto NameStrPtr = builder.CreateString("AdcReadout");
  MonitorPeakDataBuilder MonBuilder(builder);
  MonBuilder.add_Channel(Channel);
  MonBuilder.add_Name(NameStrPtr);
  MonBuilder.add_TimeStamp(TimeStamp);
  MonBuilder.add_Amplitude(Amplitude);
  builder.Finish(MonBuilder.Finish(), MonitorPeakDataIdentifier());
  ProducerPtr->produce(reinterpret_cast<char*>(builder.GetBufferPointer()), builder.GetSize());
}

ModuleAnalysisResult FindPeak(const std::vector<std::uint16_t> &SampleRun) {
  ModuleAnalysisResult ReturnData{0, 0, std::numeric_limits<std::uint16_t>::max(), 0, 0};
  std::int64_t Sum = 0;
  for (std::uint32_t i = 0; i < SampleRun.size(); ++i) {
    Sum +=SampleRun[i];
    if (SampleRun[i] > ReturnData.Max) {
      ReturnData.Max = SampleRun[i];
      ReturnData.MaxLocation = i;
    }
    if (SampleRun[i] < ReturnData.Min) {
      ReturnData.Min = SampleRun[i];
      ReturnData.MinLocation = i;
    }
  }
  ReturnData.Mean = Sum / SampleRun.size();
  return ReturnData;
}

