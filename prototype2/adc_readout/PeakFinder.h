//
//  ADC_Readout.hpp
//  ADC_Data_Receiver
//
//  Created by Jonas Nilsson on 2017-10-17.
//  Copyright © 2017 European Spallation Source. All rights reserved.
//

#pragma once

#include "AdcDataProcessor.h"

class PeakFinder : public AdcDataProcessor {
public:
  PeakFinder(std::shared_ptr<Producer> Prod, bool PositivePolarity = true);
  virtual void operator()(const PacketData &Data) override;
private:
  void SendData(const std::uint64_t &TimeStamp, const std::uint16_t &Amplitude, const std::uint16_t &Channel);
  bool PositivePulse;
};

struct ModuleAnalysisResult {
  std::uint16_t Max;
  std::uint16_t MaxLocation;
  std::uint16_t Min;
  std::uint16_t MinLocation;
  std::uint16_t Mean;
};

ModuleAnalysisResult FindPeak(const std::vector<std::uint16_t> &SampleRun);
