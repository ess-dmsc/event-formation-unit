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
#include <common/ev42_events_generated.h>

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
  auto SourceName = builder.CreateString("adc_channel_" + std::to_string(Channel));
  auto ToF_Vector = builder.CreateVector(std::vector<std::uint32_t>{0});
  auto AmplitudeVector = builder.CreateVector(std::vector<std::uint32_t>{Amplitude});
  EventMessageBuilder EvBuilder(builder);
  EvBuilder.add_source_name(SourceName);
  EvBuilder.add_message_id(EventCounter++);
  EvBuilder.add_pulse_time(TimeStamp);
  EvBuilder.add_time_of_flight(ToF_Vector);
  EvBuilder.add_detector_id(AmplitudeVector);
  builder.Finish(EvBuilder.Finish(), EventMessageIdentifier());
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

