// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial CAEN readouts
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <cstdint>
#include <modules/caen/generators/ReadoutGenerator.h>

namespace Caen {

ReadoutGenerator::ReadoutGenerator() : ReadoutGeneratorBase(ESSReadout::Parser::DetectorType::BIFROST) {
  app.add_option("--detector", CaenSettings.Detector,
                "Specify detector name (LOKI, CSPEC, ..)");
  app.add_flag("--tof", CaenSettings.Tof,
                "generate tof distribution");
  app.add_flag("--loki", CaenSettings.Loki,
                "generate data for all four amplitudes");
  app.add_option("--fibermask", CaenSettings.FiberMask,
                "Mask out unused fibers");
  app.add_option("--fenmask", CaenSettings.FENMask,
                "Mask out unused FENs");
  app.add_option("--groupmask", CaenSettings.GroupMask,
                "Mask out unused Groupss");
}


uint8_t ReadoutGenerator::get8(int MaxVal, int Mask) {
  while (true) {
    uint8_t Id = Fuzzer.random8() % MaxVal;
    int val = 1 << Id;
    if (val & Mask) {
      return Id;
    }
  }
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



  ReadoutData.FiberId = get8(23, CaenSettings.FiberMask);
  ReadoutData.FENId = get8(12, CaenSettings.FENMask);
  ReadoutData.Group = get8(15, CaenSettings.GroupMask);
  ReadoutData.AmpA = Fuzzer.random16();
  ReadoutData.AmpB = Fuzzer.random16();

  if (CaenSettings.Loki) {
    ReadoutData.AmpC = Fuzzer.random16();
    ReadoutData.AmpD = Fuzzer.random16();
  } else {
    ReadoutData.AmpC = 0;
    ReadoutData.AmpD = 0;
  }


  printf("fiber %2u, fen %2u, timehi %10u, timelo %10u, group %2u, a: %5u, b: %5u, c: %5u, d: %5u\n",
      ReadoutData.FiberId, ReadoutData.FENId,
      ReadoutData.TimeHigh, ReadoutData.TimeLow,
      ReadoutData.Group,
      (uint16_t)ReadoutData.AmpA, (uint16_t)ReadoutData.AmpB,
      (uint16_t)ReadoutData.AmpC, (uint16_t)ReadoutData.AmpD);

  return true;
}

/// \brief implementation of virtual functio from base class
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

} // namespace Dream
// GCOVR_EXCL_STOP
