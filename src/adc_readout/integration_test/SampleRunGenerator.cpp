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

SampleRunGenerator::SampleRunGenerator(size_t Samples, double PeakPos,
                                       double PeakSigma, double Slope,
                                       double Offset, int ADCBox,
                                       int ADCChannel)
    : Buffer(Samples * sizeof(std::uint16_t) + sizeof(DataHeader) + 4, 0xED),
      NrOFSamples(Samples), PeakLocation(PeakPos), PeakWidth(PeakSigma),
      BkgSlope(Slope), BkgOffset(Offset), ADCBoxNr(ADCBox),
      ADCChannelNr(ADCChannel), PeakBuffer(Samples) {

  // Set-up sample run header and trailer
  HeaderPtr()->Channel = ADCChannelNr;
  HeaderPtr()->MagicValue = 0xABCD;
  HeaderPtr()->Oversampling = 1;
  HeaderPtr()->Length =
      Samples * sizeof(std::uint16_t) + sizeof(DataHeader) + 4;
  HeaderPtr()->fixEndian();
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
  HeaderPtr()->TimeStamp = {Time.getSeconds(), Time.getSecondsFrac()};
  HeaderPtr()->TimeStamp.fixEndian();

  std::uint16_t *samplePtr = SamplePtr();

  if (0) {
    for (auto i = 0u; i < NrOFSamples; ++i) {
      samplePtr[i] = htons(
          std::lround(PeakBuffer[i] * Amplitude + BkgSlope * i + BkgOffset));
    }
  } else {
    int i32_NumSamples = (int)NrOFSamples;
    float f32_Amplitude = (float)Amplitude;
    float f32_BkgSlope = (float)BkgSlope;
    float f32_BkgOffset = (float)BkgOffset;
    float f32_i_mul_BkgSlope_plus_BkgOffset = f32_BkgOffset;

    for (int i = 0; i < i32_NumSamples; ++i) {
      samplePtr[i] = htons(std::lround(PeakBuffer[i] * f32_Amplitude +
                                       f32_i_mul_BkgSlope_plus_BkgOffset));
      f32_i_mul_BkgSlope_plus_BkgOffset += f32_BkgSlope;
    }
  }

  return std::make_pair(Buffer.data(), NrOFSamples * sizeof(std::uint16_t) +
                                           sizeof(DataHeader) + 4);
}
