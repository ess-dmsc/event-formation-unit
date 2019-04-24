/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Simple peak finding implementation.
 */

#include "PeakFinder.h"
#include "PulseParameters.h"
#include "ev42_events_generated.h"
#include <cmath>
#include <limits>

PeakFinder::PeakFinder(std::shared_ptr<Producer> Prod, std::string SourceName)
    : AdcDataProcessor(std::move(Prod)), Name(std::move(SourceName)) {}

void PeakFinder::processData(SamplingRun const &Data) {
  auto PulseInfo = analyseSampleRun(Data, 0.1);
  std::uint64_t PeakTimeStamp = PulseInfo.PeakTimestamp.GetTimeStampNS();
  sendData(PeakTimeStamp, PulseInfo.PeakAmplitude, Data.Identifier);
}

void PeakFinder::sendData(const std::uint64_t &TimeStamp,
                          const std::uint16_t &Amplitude,
                          const ChannelID &Identifier) {
  flatbuffers::FlatBufferBuilder builder;
  auto SourceName =
      builder.CreateString(Name + "_Adc" + std::to_string(Identifier.SourceID) +
                           "_Ch" + std::to_string(Identifier.ChannelNr));
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
#pragma message("Use of Producer::produce() must be corrected to not use system time.")
  ProducerPtr->produce({builder.GetBufferPointer(),
                       builder.GetSize()}, time(nullptr) * 1000);
}
