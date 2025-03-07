// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial VMM3 readouts for multi-blade based setups
//===----------------------------------------------------------------------===//

// GCOVR_EXCL_START

#include <modules/freia/generators/MultiBladeGenerator.h>
#include <common/readout/vmm3/VMM3Parser.h>

#include <fmt/core.h>
#include <cmath>

namespace Freia {


// Values must be > 0 and <= 32, Mask must be nonzero
uint8_t MultiBladeGenerator::randU8WithMask(int Range, int Mask) {
  while (true) {
    uint8_t Id = Fuzzer.random8() % Range;
    int BitVal = 1 << Id;
    if (BitVal & Mask) {
      return Id;
    }
    // repeat until match
  }
}

MultiBladeGenerator::MultiBladeGenerator() : ReadoutGeneratorBase(ESSReadout::Parser::DetectorType::FREIA) {
  // app.add_option("--detector", MultiBladeSettings.Detector,
  //         "Specify detector name (LOKI, CSPEC, ..)")->required();

  app.add_option("--fibervals",  MultiBladeSettings.FiberVals, "Number of Fiber values to generate");
  app.add_option("--fibermask",  MultiBladeSettings.FiberMask, "Mask out unused fibers");
 
  app.add_option("--fenvals",    MultiBladeSettings.FENVals, "Number of FEN values to generate");
  app.add_option("--fenmask",    MultiBladeSettings.FENMask, "Mask out unused FENs");

  app.add_option("--hybridvals", MultiBladeSettings.HybridVals, "Number of Hybrid values to generate");
  app.add_option("--hybridmask", MultiBladeSettings.HybridMask, "Mask out unused Hybrids");

  // app.add_option("--groupmask", MultiBladeSettings.GroupMask,
  //         "Mask out unused Groupss");
  // app.add_option("--groupvals", MultiBladeSettings.GroupVals,
  //         "Number of Group values to generate");

}

void MultiBladeGenerator::generateData() {
  double Angle{0};
  double XChannel{32};
  double YChannel{32};

  constexpr double DEG_TO_RADS = M_PI/180.0;
  constexpr size_t DATA_LENGTH = sizeof(VMM3Data);

  for (size_t Count = 0; Count < numberOfReadouts; Count++) {
    // Get a VMM3Data struct pointer for the next Buffer write position
    VMM3Data * ReadoutData = getReadoutDataPtr(Count);

    // Set readout data
    ReadoutData->FiberId = randU8WithMask(MultiBladeSettings.FiberVals, MultiBladeSettings.FiberMask);
    ReadoutData->FENId   = randU8WithMask(MultiBladeSettings.FENVals,   MultiBladeSettings.FENMask);

    ReadoutData->DataLength = DATA_LENGTH;
    assert(ReadoutData->DataLength == 20);

    ReadoutData->TimeHigh = getReadoutTimeHigh();
    ReadoutData->TimeLow  = getReadoutTimeLow();

    
    // We number consecutive the VMMs as 0, 1, 2, and 3
    const uint8_t VMM = Count & 0x3;

    // Consecutive VMM pairs are accepted as 
    // 
    //    Mask        VMM Pair(s) 
    //   3 - 0x3   [0, 1]
    //  12 - 0xc   [2, 3]
    //  15 - 0xf   [0, 1] and [2, 3]
    //
    // Check if a pair passes the mask 
    if ((1 << VMM) & ~MultiBladeSettings.HybridMask) {
      continue;
    }

    ReadoutData->VMM = VMM;
    fmt::print("VMM = {} {}\n", Count & 0x3, 1 << (Count & 0x3));
    ReadoutData->OTADC = 1000;

    if ((Count % 2) == 0) {
      Angle = 360.0 * Fuzzer.random8() / 255.0;
      XChannel = 54.0 - 15 * ReadoutData->FiberId + 5.0 * cos(Angle * DEG_TO_RADS);
      YChannel = 30.0 + 5.0 * sin(Angle * DEG_TO_RADS);
      ReadoutData->Channel = YChannel;
    }

    else {
      ReadoutData->Channel = XChannel;
    }

    if ((Count % 2) == 0) {
      addTicksBtwReadoutsToReadoutTime();
    }

    else {
      addTickBtwEventsToReadoutTime();
    }
  }
}

VMM3Data *MultiBladeGenerator::getReadoutDataPtr(size_t Index) {
  return (VMM3Data *) &Buffer[HeaderSize + Index * ReadoutDataSize];
}

} // namespace Freia

// GCOVR_EXCL_STOP
