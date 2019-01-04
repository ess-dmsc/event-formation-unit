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

/// \brief Finds a peak in a sample run and serialises its maximum value and
/// timestamp.
class PeakFinder : public AdcDataProcessor {
public:
  /// \param[in] Prod A shared pointer to the Kafka producer that handles data
  /// production.
  PeakFinder(std::shared_ptr<Producer> Prod, std::string SourceName);

  /// \brief Handles peak detection, serialisation of the result and
  /// transmission to the Kafka broker.
  virtual void processData(SamplingRun const &Data) override;

private:
  /// \brief Implements serialisation and transmission of the peak data.
  /// Currently uses the EventMessage flatbuffer schema for serialisation. This
  /// is not a good fit for the analysed data and another schema should probably
  /// be used.
  void sendData(const std::uint64_t &TimeStamp, const std::uint16_t &Amplitude,
                const ChannelID &Identifier);
  std::uint64_t EventCounter{0};
  std::string Name;
};
