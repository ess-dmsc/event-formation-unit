/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Simple peak finding algorithm implementation header.
///
//===----------------------------------------------------------------------===//

#pragma once

#include "AdcDataProcessor.h"
#include "EventSerializer.h"
#include <map>

/// \brief Finds a peak in a sample run and serialises its maximum value and
/// timestamp.
class PeakFinder : public AdcDataProcessor {
public:
  /// \param[in] Prod A shared pointer to the Kafka producer that handles data
  /// production.
  PeakFinder(std::shared_ptr<Producer> Prod, std::string SourceName,
             OffsetTime RefTimeOffset);

  /// \brief Handles peak detection, serialisation of the result and
  /// transmission to the Kafka broker.
  virtual void processData(SamplingRun const &Data) override;

private:
  std::string Name;
  std::map<ChannelID, std::unique_ptr<EventSerializer>> Serialisers;
  OffsetTime UsedTimeOffset;
};
