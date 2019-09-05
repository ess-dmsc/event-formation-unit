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
    : Buffer(new std::uint8_t[Samples * sizeof(std::uint16_t) +
                              sizeof(DataHeader) + 4]),
      HeaderPtr(reinterpret_cast<DataHeader *>(Buffer.get())),
      SamplePtr(
          reinterpret_cast<std::uint16_t *>(Buffer.get() + sizeof(DataHeader))),
      NrOFSamples(Samples), PeakLocation(PeakPos), PeakWidth(PeakSigma),
      BkgSlope(Slope), BkgOffset(Offset), ADCBoxNr(ADCBox),
      ADCChannelNr(ADCChannel), PeakBuffer(Samples) {

  // Set-up sample run header and trailer
  HeaderPtr->Channel = ADCChannelNr;
  HeaderPtr->MagicValue = 0xABCD;
  HeaderPtr->Oversampling = 1;
  HeaderPtr->Length = Samples * sizeof(std::uint16_t) + sizeof(DataHeader) + 4;
  HeaderPtr->fixEndian();
  auto TrailerPtr = reinterpret_cast<std::uint32_t *>(
      Buffer.get() + Samples * sizeof(std::uint16_t) + sizeof(DataHeader));
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
SampleRunGenerator::generate(double Amplitude, RawTimeStamp const Time) {
  HeaderPtr->TimeStamp = Time;
  HeaderPtr->TimeStamp.fixEndian();
  for (auto i = 0u; i < NrOFSamples; ++i) {
    SamplePtr[i] = htons(
        std::lround(PeakBuffer[i] * Amplitude + BkgSlope * i + BkgOffset));
  }
  return std::make_pair(Buffer.get(), NrOFSamples * sizeof(std::uint16_t) +
                                          sizeof(DataHeader) + 4);
}
