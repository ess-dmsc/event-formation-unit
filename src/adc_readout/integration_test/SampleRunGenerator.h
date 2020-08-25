/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include "../AdcParse.h"
#include "../AdcTimeStamp.h"
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

class SampleRunGenerator {
public:
  SampleRunGenerator(std::size_t Samples, double PeakPos, double PeakSigma,
                     double Slope, double Offset, int ADCBox, int ADCChannel);
  std::pair<void *, std::size_t> generate(double Amplitude,
                                          TimeStamp const Time);

private:
  DataHeader *GetHeaderPtr() {
    return reinterpret_cast<DataHeader *>(Buffer.data());
  }

  std::uint16_t *GetSamplePtr() {
    return reinterpret_cast<std::uint16_t *>(Buffer.data() +
                                             sizeof(DataHeader));
  }

  std::vector<std::uint8_t> Buffer;
  std::size_t NrOFSamples{50};
  double PeakLocation{25};
  double PeakWidth{10};
  double BkgSlope{0};
  double BkgOffset{0};
  int ADCBoxNr{0};
  int ADCChannelNr{0};
  std::vector<float> PeakBuffer;
};
