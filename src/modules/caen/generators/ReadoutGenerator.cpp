// Copyright (C) 2024 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial CAEN readouts
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <modules/caen/generators/ReadoutGenerator.h>
#include <common/testutils/bitmaps/BitMaps.h>

#include <fmt/core.h>

#include <cstdint>

namespace Caen {

ReadoutGenerator::ReadoutGenerator() : ReadoutGeneratorBase(ESSReadout::Parser::DetectorType::Reserved) {
  // clang-format off

  // Options
  app.add_option("--detector", CaenSettings.Detector,
                "Specify detector name (LOKI, CSPEC, ..)")->required();

  app.add_option("--fibervals", CaenSettings.FiberVals, "Number of Fiber values to generate");
  app.add_option("--fibermask", CaenSettings.FiberMask, "Mask out unused fibers");

  app.add_option("--fenvals", CaenSettings.FENVals, "Number of FEN values to generate");
  app.add_option("--fenmask", CaenSettings.FENMask, "Mask out unused FENs");

  app.add_option("--groupvals", CaenSettings.GroupVals, "Number of Group values to generate");
  app.add_option("--groupmask", CaenSettings.GroupMask, "Mask out unused Groups");

  // Flags
  app.add_flag("--loki",    CaenSettings.Loki,    "generate data for all four amplitudes");
  app.add_flag("--bitmaps", CaenSettings.Bitmaps, "generate data using bitmap images");

  // clang-format on
}


void ReadoutGenerator::generateRandomData() {
  for (size_t Count = 0; Count < Settings.NumReadouts; Count++) {
    DataParser::CaenReadout &ReadoutData = *getReadoutDataPtr(Count);
    ReadoutData.DataLength = ReadoutDataSize;

    if (Settings.Tof) {
      double TofMs = TofDist.getValue();
      ReadoutData.TimeHigh = getPulseTimeHigh();
      ReadoutData.TimeLow = getPulseTimeLow() + static_cast<uint32_t>(TofMs * TicksPerMs);
    } else {
      ReadoutData.TimeHigh = getReadoutTimeHigh();
      ReadoutData.TimeLow = getReadoutTimeLow();
    }

    ReadoutData.FlagsOM = 0;

    ReadoutData.FiberId = Fuzzer.randU8WithMask(CaenSettings.FiberVals, CaenSettings.FiberMask);
    ReadoutData.FENId = Fuzzer.randU8WithMask(CaenSettings.FENVals, CaenSettings.FENMask);
    ReadoutData.Group = Fuzzer.randU8WithMask(CaenSettings.GroupVals, CaenSettings.GroupMask);
    ReadoutData.AmpA = Fuzzer.random16() & CaenSettings.AmplitudeMask;
    ReadoutData.AmpB = Fuzzer.random16() & CaenSettings.AmplitudeMask;

    if (CaenSettings.Loki) {
      ReadoutData.AmpC = Fuzzer.random16() & CaenSettings.AmplitudeMask;
      ReadoutData.AmpD = Fuzzer.random16() & CaenSettings.AmplitudeMask;
    } else {
      ReadoutData.AmpC = 0;
      ReadoutData.AmpD = 0;
    }

    if (Settings.Debug) {
      printDebug(ReadoutData);
    }
  }
}

void ReadoutGenerator::generateMaskedData() {
  // Check amplitude algorithm for all valid straw and position combos
  if (Settings.Debug) {
    testAmplitudes();
  }

  // clang-format off
  // Load bitmaps to produce a message
  using Bitmap = BitMaps::Bitmap;
  std::map<size_t, Bitmap> bitmaps;
  for (const auto &bitmap: {
    Bitmap(BitMaps::Space),
    Bitmap(BitMaps::B),
    Bitmap(BitMaps::E),
    Bitmap(BitMaps::A),
    Bitmap(BitMaps::M),
    Bitmap(BitMaps::Space),
    Bitmap(BitMaps::O),
    Bitmap(BitMaps::N),
    Bitmap(BitMaps::Space),
    Bitmap(BitMaps::T),
    Bitmap(BitMaps::A),
    Bitmap(BitMaps::R),
    Bitmap(BitMaps::G),
    Bitmap(BitMaps::E),
    Bitmap(BitMaps::T),
    Bitmap(BitMaps::Space),
    Bitmap(BitMaps::N2),
    Bitmap(BitMaps::N5),
    Bitmap(BitMaps::Space),
  }) {
    bitmaps[bitmaps.size()] = bitmap;
  }
  // clang-format on

  // Generate all readouts
  for (size_t Count = 0; Count < Settings.NumReadouts; Count++) {
    DataParser::CaenReadout &ReadoutData = *getReadoutDataPtr(Count);
    ReadoutData.DataLength = ReadoutDataSize;

    if (Settings.Tof) {
      double TofMs = TofDist.getValue();
      ReadoutData.TimeHigh = getPulseTimeHigh();
      ReadoutData.TimeLow = getPulseTimeLow() + static_cast<uint32_t>(TofMs * TicksPerMs);
    } else {
      ReadoutData.TimeHigh = getReadoutTimeHigh();
      ReadoutData.TimeLow = getReadoutTimeLow();
    }

    ReadoutData.FlagsOM = 0;

    ReadoutData.FiberId = Fuzzer.randU8WithMask(CaenSettings.FiberVals, CaenSettings.FiberMask);
    ReadoutData.FENId = Fuzzer.randU8WithMask(CaenSettings.FENVals, CaenSettings.FENMask);
    ReadoutData.Group = Fuzzer.randU8WithMask(CaenSettings.GroupVals, CaenSettings.GroupMask);
    ReadoutData.AmpA = Fuzzer.random16() & CaenSettings.AmplitudeMask;
    ReadoutData.AmpB = Fuzzer.random16() & CaenSettings.AmplitudeMask;

    if (CaenSettings.Loki) {
      ReadoutData.AmpC = Fuzzer.random16() & CaenSettings.AmplitudeMask;
      ReadoutData.AmpD = Fuzzer.random16() & CaenSettings.AmplitudeMask;
    } else {
      ReadoutData.AmpC = 0;
      ReadoutData.AmpD = 0;
    }

    // Determine the 7-pixel band
    const bool isFENEven = (ReadoutData.FENId % 2 == 0);
    const bool isLowerGroup = ReadoutData.Group < 4;
    size_t offset = isLowerGroup ? 0 : 7;
    offset += isFENEven ? 0 : 14;

    // Get random straw and position number
    size_t s = Fuzzer.randomInterval(0, 7);
    size_t p = Fuzzer.randomInterval(0, 512);

    // Check if pixel should be drawn
    const size_t x = p % 28;
    const size_t y = s + offset;
    const size_t index = (p / 28) % bitmaps.size();
    if (bitmaps[index].get(x, y) == bmp::White) {
      s = 0;
      p = 0;
    }

    // Calculate four amplitudes for the given straw and position
    auto [A, B, C, D] = amplitudes(s, p);
    ReadoutData.AmpA = A & CaenSettings.AmplitudeMask;
    ReadoutData.AmpB = B & CaenSettings.AmplitudeMask;
    ReadoutData.AmpC = C & CaenSettings.AmplitudeMask;
    ReadoutData.AmpD = D & CaenSettings.AmplitudeMask;

    if (Settings.Debug) {
      printDebug(ReadoutData);
    }

    addTickBtwEventsToReadoutTime();
  }
}

void ReadoutGenerator::printDebug(const DataParser::CaenReadout &ReadoutData) {
  printf("fiber %2u, fen %2u, timehi %10u, timelo %10u, group %2u, A: %5u, B: %5u, C: %5u, D: %5u\n",
      ReadoutData.FiberId, ReadoutData.FENId,
      ReadoutData.TimeHigh, ReadoutData.TimeLow,
      ReadoutData.Group,
      static_cast<uint16_t>(ReadoutData.AmpA), static_cast<uint16_t>(ReadoutData.AmpB),
      static_cast<uint16_t>(ReadoutData.AmpC), static_cast<uint16_t>(ReadoutData.AmpD));
}

void ReadoutGenerator::generateData()
{
    if (CaenSettings.Bitmaps) {
      generateMaskedData();
    } else {
    generateRandomData();
  }
}

void Caen::ReadoutGenerator::testAmplitudes() const {
  bool bad = false;
  for (size_t s=0; s<7; ++s) {
    for (size_t p=0; p<512; ++p) {
      auto [a, b, c, d] = amplitudes(s, p);

      size_t s1 = static_cast<size_t>(round(straw(a, b, c, d)));
      size_t p1 = static_cast<size_t>(round(pos(a, b, c, d)));
      if (s != s1) {
        bad = true;
        fmt::print("Bad straw {} {}\n", s, s1);
      }
      if (p != p1) {
        bad = true;
        fmt::print("Bad position {} {}\n", p, p1);
      }
    }
  }

  if (bad) {
    exit(-1);
  }
}

std::tuple<int16_t, int16_t, int16_t, int16_t> ReadoutGenerator::amplitudes(size_t s, size_t p) const {
  // Normalize id and position to [0; 1]
  const double q = s / 6.0;
  const double r = p / 511.0;

  // Amplitudes vars
  double a{0};
  double b{0};
  double c{0};
  double d{0};

  // Determine the amplitudes
  if (q + r < 1) {
    b = std::min(q, r);
    d = q - b;
    a = r - b;
    c = abs(1.0 - a - b - d);
  } else {
    b = q + r - 1;
    d = q - b;
    a = r - b;
    c = 0;
  }

  // Scale amplitude into the range [0; 2^15 - 1]
  const double scale  = 32767;
  const int16_t A = static_cast<int16_t>(scale * a);
  const int16_t B = static_cast<int16_t>(scale * b);
  const int16_t C = static_cast<int16_t>(scale * c);
  const int16_t D = static_cast<int16_t>(scale * d);

  return std::make_tuple(A, B, C, D);
}

double ReadoutGenerator::straw(int16_t A, int16_t B, int16_t C, int16_t D) const {
  const double s = 6.0 * (B + D) / (A + B + C + D);

  return s;
}

double ReadoutGenerator::pos(int16_t A, int16_t B, int16_t C, int16_t D) const {
  const double p = 511.0 * (A + B) / (A+ B + C + D);

  return p;
}

DataParser::CaenReadout *ReadoutGenerator::getReadoutDataPtr(size_t Index) {
  return (DataParser::CaenReadout *)&Buffer[HeaderSize + Index * ReadoutDataSize];
}

} // namespace Dream
// GCOVR_EXCL_STOP
