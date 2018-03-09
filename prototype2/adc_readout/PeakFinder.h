/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Simple peak finding algorithm implementation header.
 */

#pragma once

#include "AdcDataProcessor.h"

class PeakFinder : public AdcDataProcessor {
public:
  PeakFinder(std::shared_ptr<Producer> Prod);
  virtual void operator()(const PacketData &Data) override;

private:
  void SendData(const std::uint64_t &TimeStamp, const std::uint16_t &Amplitude,
                const std::uint16_t &Channel);
  std::uint64_t EventCounter{0};
};

struct ModuleAnalysisResult {
  std::uint16_t Max;
  std::uint16_t MaxLocation;
  std::uint16_t Min;
  std::uint16_t MinLocation;
  std::uint16_t Mean;
};

ModuleAnalysisResult FindPeak(const std::vector<std::uint16_t> &SampleRun);
