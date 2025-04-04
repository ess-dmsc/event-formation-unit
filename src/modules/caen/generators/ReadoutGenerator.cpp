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

#include <cstdint>

namespace Caen {

ReadoutGenerator::ReadoutGenerator() : ReadoutGeneratorBase(ESSReadout::Parser::DetectorType::Reserved) {
  app.add_option("--detector", CaenSettings.Detector,
                "Specify detector name (LOKI, CSPEC, ..)")->required();
  app.add_flag("--tof", CaenSettings.Tof,
                "generate tof distribution");

  app.add_flag("--loki", CaenSettings.Loki,
                "generate data for all four amplitudes");

  // Fibermask option is already handled by the base class - see ReadoutGenerator::main
  app.add_option("--fibervals", CaenSettings.FiberVals,
                "Number of Fiber values to generate");

  app.add_option("--fenmask", CaenSettings.FENMask,
                "Mask out unused FENs");
  app.add_option("--fenvals", CaenSettings.FENVals,
                "Number of FEN values to generate");

  app.add_option("--groupmask", CaenSettings.GroupMask,
                "Mask out unused Groupss");
  app.add_option("--groupvals", CaenSettings.GroupVals,
                "Number of Group values to generate");

  app.add_flag("--debug", CaenSettings.Debug,
                "print debug information");
}

bool ReadoutGenerator::getRandomReadout(DataParser::CaenReadout &ReadoutData) {
  ReadoutData.DataLength = ReadoutDataSize;

  if (CaenSettings.Tof) {
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

  if (CaenSettings.Debug) {
    printf("fiber %2u, fen %2u, timehi %10u, timelo %10u, group %2u, a: %5u, b: %5u, c: %5u, d: %5u\n",
        ReadoutData.FiberId, ReadoutData.FENId,
        ReadoutData.TimeHigh, ReadoutData.TimeLow,
        ReadoutData.Group,
        (uint16_t)ReadoutData.AmpA, (uint16_t)ReadoutData.AmpB,
        (uint16_t)ReadoutData.AmpC, (uint16_t)ReadoutData.AmpD);
  }

  return true;
}

/// \brief implementation of virtual function from base class
void ReadoutGenerator::generateData() {
  auto DataPtr = (uint8_t *)Buffer;
  DataPtr += HeaderSize;

  uint32_t Readouts{0};
  DataParser::CaenReadout ReadoutData;

  while (Readouts < Settings.NumReadouts) {
    bool Valid = getRandomReadout(ReadoutData);
    if (not Valid) {
      continue;
    }
    memcpy(DataPtr, &ReadoutData, ReadoutDataSize);
    DataPtr += ReadoutDataSize;

    // Increment the time for next readout
    addTickBtwEventsToReadoutTime();
    Readouts++;
  }
}

void ReadoutGenerator::main() {
  // Call base class and extract the fiber related CLI values
  ReadoutGeneratorBase::main();

  // Get the fiber mask
  if (app.get_option("--fibermask") != nullptr) {
    CaenSettings.FiberMask = Settings.FiberMask;
  }
}


} // namespace Dream
// GCOVR_EXCL_STOP
