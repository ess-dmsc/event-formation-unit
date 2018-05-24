/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Simple peak finding implementation.
 */

#include "PeakFinder.h"
#include "ev42_events_generated.h"
#include <cmath>
#include <limits>

PeakFinder::PeakFinder(std::shared_ptr<Producer> Prod)
    : AdcDataProcessor(std::move(Prod)) {}

void PeakFinder::processData(DataModule const &Data) {
  auto Result = FindPeak(Data.Data);
  std::uint64_t PeakTimeStamp = Data.TimeStamp.GetOffsetTimeStamp(Result.MaxLocation).GetTimeStampNS();
    SendData(PeakTimeStamp, Result.Max, Data.Channel);
}

void PeakFinder::SendData(const std::uint64_t &TimeStamp,
                          const std::uint16_t &Amplitude,
                          const std::uint16_t &Channel) {
  flatbuffers::FlatBufferBuilder builder;
  auto SourceName =
      builder.CreateString("adc_channel_" + std::to_string(Channel));
  auto ToF_Vector = builder.CreateVector(std::vector<std::uint32_t>{0});
  auto AmplitudeVector =
      builder.CreateVector(std::vector<std::uint32_t>{Amplitude});
  EventMessageBuilder EvBuilder(builder);
  EvBuilder.add_source_name(SourceName);
  EvBuilder.add_message_id(EventCounter++);
  EvBuilder.add_pulse_time(TimeStamp);
  EvBuilder.add_time_of_flight(ToF_Vector);
  EvBuilder.add_detector_id(AmplitudeVector);
  builder.Finish(EvBuilder.Finish(), EventMessageIdentifier());
  ProducerPtr->produce(reinterpret_cast<char *>(builder.GetBufferPointer()),
                       builder.GetSize());
}

ModuleAnalysisResult FindPeak(std::vector<std::uint16_t> const &SampleRun) {
  ModuleAnalysisResult ReturnData{
      0, 0, 0};
  std::int64_t Sum = 0;
  for (std::size_t i = 0; i < SampleRun.size(); ++i) {
    Sum += SampleRun[i];
    if (SampleRun[i] > ReturnData.Max) {
      ReturnData.Max = SampleRun[i];
      ReturnData.MaxLocation = i;
    }
  }
  ReturnData.Mean = Sum / SampleRun.size();
  return ReturnData;
}
