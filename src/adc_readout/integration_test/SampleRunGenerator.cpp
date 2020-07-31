/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include "SampleRunGenerator.h"
#include <algorithm>
#include <ciso646>
#include <cmath>

static const double Pi{3.141592653589793};

double gaussian(int i, double PeakPos, double Width) {
  auto E_2 = (i - PeakPos) * (i - PeakPos);
  return std::exp(-E_2 / (2.0 * Width * Width)) /
         std::sqrt(2 * Pi * Width * Width);
}

#ifdef BUILD_SUPPORT_ISPC
static const bool IspcEnabled = true;
#else
static const bool IspcEnabled = false;
#endif

extern "C" void SampleGenIspc(uint16_t output[], int count, float PeakBuffer[],
                              float Amplitude, float BkgSlope, float BkgOffset);

SampleRunGenerator::SampleRunGenerator(size_t Samples, double PeakPos,
                                       double PeakSigma, double Slope,
                                       double Offset, int ADCBox,
                                       int ADCChannel)
    : Buffer(Samples * sizeof(std::uint16_t) + sizeof(DataHeader) + 4, 0xED),
      NrOFSamples(Samples), PeakLocation(PeakPos), PeakWidth(PeakSigma),
      BkgSlope(Slope), BkgOffset(Offset), ADCBoxNr(ADCBox),
      ADCChannelNr(ADCChannel), PeakBuffer(Samples) {

  // Set-up sample run header and trailer
  GetHeaderPtr()->Channel = ADCChannelNr;
  GetHeaderPtr()->MagicValue = 0xABCD;
  GetHeaderPtr()->Oversampling = 1;
  GetHeaderPtr()->Length =
      Samples * sizeof(std::uint16_t) + sizeof(DataHeader) + 4;
  GetHeaderPtr()->fixEndian();
  auto TrailerPtr = reinterpret_cast<std::uint32_t *>(
      Buffer.data() + Samples * sizeof(std::uint16_t) + sizeof(DataHeader));
  *TrailerPtr = htonl(0xBEEFCAFEu);

  for (auto Y = 0u; Y < PeakBuffer.size(); ++Y) {
    PeakBuffer[Y] = gaussian(Y, PeakLocation, PeakWidth);
  }
  auto ValueMultiplier =
      1.0 / *std::max_element(PeakBuffer.begin(), PeakBuffer.end());
  std::for_each(PeakBuffer.begin(), PeakBuffer.end(),
                [ValueMultiplier](auto &Value) { Value *= ValueMultiplier; });

  std::uint16_t BitPattern{0};

  BitPattern += 3;

  for (auto i = 0; i <= ADCBoxNr; ++i) {
    BitPattern = BitPattern << 2;
    BitPattern += 1;
  }
  BitPattern = BitPattern << ((1 - ADCBoxNr) * 2 + 1);

  for (auto j = 0; j <= ADCChannelNr; ++j) {
    BitPattern = BitPattern << 2;
    BitPattern += 1;
  }
  BitPattern = BitPattern << (3 - ADCChannelNr) * 2;

  auto BitPatternStart = std::lround(PeakPos + 10);
  auto BitPatternEnd = NrOFSamples - 11;
  auto BitPatternWidth = BitPatternEnd - BitPatternStart;
  auto BitPatternMultiplier = BitPatternWidth / 16;
  auto BeginIterator = PeakBuffer.begin() + BitPatternStart;
  auto EndIterator = BeginIterator + 16 * BitPatternMultiplier;
  int SampleCounter{0};
  std::for_each(
      BeginIterator, EndIterator,
      [&SampleCounter, BitPatternMultiplier, BitPattern](auto &Sample) {
        std::uint16_t BitTester =
            0x8000 >> (SampleCounter / BitPatternMultiplier);
        if (BitTester & BitPattern) {
          Sample = 0.99;
        }
        ++SampleCounter;
      });
}

std::pair<void *, std::size_t>
SampleRunGenerator::generate(double Amplitude, TimeStamp const Time) {
  GetHeaderPtr()->TimeStamp = {Time.getSeconds(), Time.getSecondsFrac()};
  GetHeaderPtr()->TimeStamp.fixEndian();

  std::uint16_t *SamplePtr = GetSamplePtr();

  if (IspcEnabled) {
    SampleGenIspc(SamplePtr, (int)NrOFSamples, PeakBuffer.data(),
                  (float)Amplitude, (float)BkgSlope, (float)BkgOffset);
  } else {
    for (auto i = 0u; i < NrOFSamples; ++i) {
      SamplePtr[i] = htons(
          std::lround(PeakBuffer[i] * Amplitude + BkgSlope * i + BkgOffset));
    }
  }
  return std::make_pair(Buffer.data(), NrOFSamples * sizeof(std::uint16_t) +
                                           sizeof(DataHeader) + 4);
}
