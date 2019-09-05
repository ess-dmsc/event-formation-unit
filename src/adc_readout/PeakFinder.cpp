/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Simple peak finding implementation.
 */

#include "PeakFinder.h"
#include "PulseParameters.h"
#include <cmath>
#include <limits>

using namespace std::chrono_literals;

PeakFinder::PeakFinder(std::shared_ptr<Producer> Prod, std::string SourceName)
    : AdcDataProcessor(std::move(Prod)), Name(std::move(SourceName)) {}

void PeakFinder::processData(SamplingRun const &Data) {
  auto PulseInfo = analyseSampleRun(Data, 0.1);
  std::uint64_t PeakTimeStamp = PulseInfo.PeakTimestamp.getTimeStampNS();

  if (Serialisers.find(Data.Identifier) == Serialisers.end()) {
    Serialisers[Data.Identifier] = std::make_unique<RefFilteredEventSerializer>(
        Name + "_Adc" + std::to_string(Data.Identifier.SourceID) + "_Ch" +
            std::to_string(Data.Identifier.ChannelNr),
        100, 50ms, ProducerPtr.get(),
        EventSerializer::TimestampMode::INDEPENDENT_EVENTS);
  }
  auto &CurrentSerialiser = Serialisers[Data.Identifier];
  CurrentSerialiser->addReferenceTimestamp(
      Data.ReferenceTimestamp.getTimeStampNS());
  CurrentSerialiser->addEvent(std::unique_ptr<EventData>(new EventData{
      PeakTimeStamp, 1, static_cast<std::uint32_t>(PulseInfo.PeakAmplitude),
      static_cast<std::uint32_t>(PulseInfo.PeakArea),
      static_cast<std::uint32_t>(PulseInfo.BackgroundLevel),
      PulseInfo.ThresholdTimestamp.getTimeStampNS(), PeakTimeStamp}));
}
