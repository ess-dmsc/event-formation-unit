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

PeakFinder::PeakFinder(std::shared_ptr<Producer> Prod, std::string SourceName, OffsetTime RefTimeOffset)
    : AdcDataProcessor(std::move(Prod)), Name(std::move(SourceName)), UsedTimeOffset(RefTimeOffset) {}

void PeakFinder::processData(SamplingRun const &Data) {
  auto PulseInfo = analyseSampleRun(Data, 0.1);
  std::uint64_t PeakTimeStamp = PulseInfo.PeakTime.getTimeStampNS();

  if (Serialisers.find(Data.Identifier) == Serialisers.end()) {
    Serialisers[Data.Identifier] = std::make_unique<EventSerializer>(
        Name + "_Adc" + std::to_string(Data.Identifier.SourceID) + "_Ch" +
            std::to_string(Data.Identifier.ChannelNr),
        200, 500ms, ProducerPtr.get(),
        EventSerializer::TimestampMode::TIME_REFERENCED, UsedTimeOffset);
  }
  auto &CurrentSerialiser = Serialisers[Data.Identifier];
  CurrentSerialiser->addReferenceTimestamp(
      Data.ReferenceTimestamp.getTimeStampNS());
  constexpr std::uint32_t PixelIdOffset{512 * 512};
  auto EventId =
      PixelIdOffset + static_cast<std::uint32_t>(Data.Identifier.ChannelNr);
  CurrentSerialiser->addEvent(std::unique_ptr<EventData>(
      new EventData{PeakTimeStamp, EventId,
                    static_cast<std::uint32_t>(PulseInfo.PeakAmplitude),
                    static_cast<std::uint32_t>(PulseInfo.PeakArea),
                    static_cast<std::uint32_t>(PulseInfo.BackgroundLevel),
                    PulseInfo.ThresholdTime.getTimeStampNS(), PeakTimeStamp}));
}
