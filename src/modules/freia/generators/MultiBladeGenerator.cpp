// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial VMM3 readouts for multi-blade based setups
//===----------------------------------------------------------------------===//

// GCOVR_EXCL_START

#include <modules/freia/generators/MultiBladeGenerator.h>
#include <modules/freia/geometry/Geometry.h>
#include <common/readout/vmm3/VMM3Parser.h>

#include <fmt/core.h>
#include <cmath>

namespace Freia {

MultiBladeGenerator::MultiBladeGenerator() : ReadoutGeneratorBase(ESSReadout::Parser::DetectorType::FREIA) {
  // Options
  app.add_option("--detector", MultiBladeSettings.Detector, "Specify detector name (Freia, Estia, or AMOR)");
  app.add_option("--fens",     MultiBladeSettings.NFENs,    "Number of FENs (Front End Nodes)");
  app.add_option("--fenmask",  MultiBladeSettings.FENMask,  "Mask out unused FENs");
  app.add_option("--vmmmask",  MultiBladeSettings.VMMMask,  "Mask out unused Hybrids");

  // Flags
  app.add_flag("--tof",   MultiBladeSettings.Tof,   "Generate tof distribution");
}

void MultiBladeGenerator::generateData() {
  // Angular used for circular baseds data 
  double Theta{0};

  // Channels for upper half circle
  double X0Channel{32};
  double Y0Channel{32};

  // Channels for lower half circle
  double X1Channel{32};
  double Y1Channel{32};

  // Misc Ids
  uint8_t VMM{0};
  uint8_t FENId{0};
  uint8_t FiberId{0};
  double TofMs{0};

  constexpr size_t DATA_LENGTH = sizeof(VMM3Data);

  // We loop over all readout counts. For a given Fiber and FEN, we use two iterations to 
  // generate a channel pair lying on either 
  // 
  //   - upper semi-circle (VMM 0 and 1)
  //   - lower semi-circle (VMM 2 amd 3) 
  for (size_t Count = 0; Count < numberOfReadouts; Count++) {
    // Get a VMM3Data struct pointer for the next Buffer write position
    VMM3Data * ReadoutData = getReadoutDataPtr(Count);

    // Misc 
    ReadoutData->DataLength = DATA_LENGTH;
    ReadoutData->OTADC = 1000;
    assert(ReadoutData->DataLength == 20);

    // Get FEN and Fibers Ids + Tof
    if (Count % 2 == 0) {
      FiberId = Fuzzer.randU8WithMask(MultiBladeSettings.NFibers, MultiBladeSettings.FiberMask);
      FENId   = Fuzzer.randU8WithMask(MultiBladeSettings.NFENs,   MultiBladeSettings.FENMask);
      TofMs   = TofDist.getValue();
    }

    ReadoutData->FiberId = FiberId;
    ReadoutData->FENId   = FENId;

    // Tof or not
    if (MultiBladeSettings.Tof) {
      ReadoutData->TimeHigh = getPulseTimeHigh();
      ReadoutData->TimeLow = getPulseTimeLow() + static_cast<uint32_t>(TofMs * TicksPerMs);
    } 
    
    else {
      ReadoutData->TimeHigh = getReadoutTimeHigh();
      ReadoutData->TimeLow = getReadoutTimeLow();
    }

    // -------------------------------------------------------------------------
    // We number consecutive VMMs as 0, 1, 2, 3, 0, 1, 2, ...
    //
    // The following consecutive VMM pairs are accepted
    // 
    //    Mask        VMM Pair(s) 
    //   1 - 0x1       [0, 1]
    //   2 - 0x2       [2, 3]
    //   3 - 0x3       [0, 1] and [2, 3]
    if (Count % 2 == 0) {
      VMM =  2 * Fuzzer.randU8WithMask(2, MultiBladeSettings.VMMMask);
    }
    else {
      VMM += 1;
    }
    ReadoutData->VMM = VMM;

    // Generate circular pixel shapes
    const double R = 31;
    const double Sg = (ReadoutData->FiberId % 2) ? 1 : -1;
    const double Delta = 12;

    switch (VMM) {
      // Upper semi-circle
      case 0:
        Theta = M_PI * Fuzzer.random8() / 255.0;
        X0Channel = 63 - (31 + 0.25 * R * cos(Theta) + Sg * Delta);
        Y0Channel = 16 + R * sin(Theta);

        ReadoutData->Channel = round(Y0Channel);
        break;
      
      case 1:
        ReadoutData->Channel = round(X0Channel);
        break;

      // Lower semi-circle
      case 2:
        Theta = M_PI * Fuzzer.random8() / 255.0;
        X1Channel = 63 - (31 + 0.25 * R * cos(Theta)  + Sg * Delta);
        Y1Channel = 47 - R * sin(Theta);

        ReadoutData->Channel = round(Y1Channel);
        break;

      case 3:
        ReadoutData->Channel = round(X1Channel);
        break;

      default:
        break;
    }

    // Short time delta between correlated X- and Y-channels 
    if ((Count % 2) == 0) {
      addTicksBtwReadoutsToReadoutTime();
    }

    // Large time delta between uncorrelated X- and Y-channels 
    else {
      addTickBtwEventsToReadoutTime();
    }
  }
}

void MultiBladeGenerator::main() {
  // Call base class and extract the fiber related CLI values
  ReadoutGeneratorBase::main();

  // Get number of fibers and the associated mask
  if (app.get_option("--fibers") != nullptr) {
    MultiBladeSettings.NFibers = Settings.NFibers;
  }

  if (app.get_option("--fibermask") != nullptr) {
    MultiBladeSettings.FiberMask = Settings.FiberMask;
  }

  // Set detector type
  setDetectorType(MultiBladeSettings.Detector);

  // Extract instrument geometry
  Geometry Geom;
  Geom.setGeometry(MultiBladeSettings.Detector);
  XDim = Geom.xDim();
  YDim = Geom.yDim();
}

VMM3Data *MultiBladeGenerator::getReadoutDataPtr(size_t Index) {
  return (VMM3Data *) &Buffer[HeaderSize + Index * ReadoutDataSize];
}

} // namespace Freia

// GCOVR_EXCL_STOP
