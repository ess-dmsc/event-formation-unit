/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Simple peak finding algorithm implementation header.
 */

#pragma once

#include "AdcDataProcessor.h"

/// @brief Finds a peak in a sample run and serialises its maximum value and
/// timestamp.
class PeakFinder : public AdcDataProcessor {
public:
  /// @param[in] Prod A shared pointer to the Kafka producer that handles data
  /// production.
  PeakFinder(std::shared_ptr<Producer> Prod);

  /// @brief Handles peak detection, serialisation of the result and
  /// transmission to the Kafka broker.
  virtual void processData(DataModule const &Data) override;

private:
  /// @brief Implements serialisation and transmission of the peak data.
  /// Currently uses the EventMessage flatbuffer schema for serialisation. This
  /// is not a good fit for the analysed data and another schema should probably
  /// be used.
  void SendData(const std::uint64_t &TimeStamp, const std::uint16_t &Amplitude,
                const std::uint16_t &Channel);
  std::uint64_t EventCounter{0};
};

struct ModuleAnalysisResult {
  std::uint16_t Max;
  std::uint16_t MaxLocation;
  std::uint16_t Mean;
};

/// @brief Implements the actual peak finding algorithm
/// The algorithm is very simple (to a fault) and only attempts to find the
/// maximum value in a sampling run and then also calculates the timestamp of
/// that maximum value. This algorithm will fail for doubles, exessive noise,
/// wide peaks, valleys (inverted peaks) and probably other situations as well.
/// @param[in] SampleRun A vector of samples to find the peak in.
/// @return The results of the peak finding algorithm.
ModuleAnalysisResult FindPeak(const std::vector<std::uint16_t> &SampleRun);
