// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial VMM3 readouts for multi-blade based setups
//===----------------------------------------------------------------------===//

// GCOVR_EXCL_START

#include <modules/freia/generators/MultiBladeGenerator.h>
#include <modules/freia/generators/BitMaps.h>
#include <modules/freia/geometry/Geometry.h>
#include <common/readout/vmm3/VMM3Parser.h>

#include <fmt/core.h>
#include <cmath>

using namespace ESSReadout;

namespace Freia {

MultiBladeGenerator::MultiBladeGenerator() : ReadoutGeneratorBase(ESSReadout::Parser::DetectorType::FREIA) {
  // Options
  app.add_option("--detector", MultiBladeSettings.Detector, "Specify detector name (Freia or Estia)");

  app.add_option("--fens",     MultiBladeSettings.NFENs,    "Number of FENs (Front End Nodes)");
  app.add_option("--fenmask",  MultiBladeSettings.FENMask,  "Mask out unused FENs");

  app.add_option("--vmms",     MultiBladeSettings.NVMMs,    "Number of VMMs (Hybrids)");
  app.add_option("--vmmmask",  MultiBladeSettings.VMMMask,  "Mask out unused VMMs");

  // Flags
  app.add_flag("--tof",   MultiBladeSettings.Tof,   "Generate tof distribution");
  app.add_flag("--debug", MultiBladeSettings.Debug, "Print debug info");

  // Load bitmaps
  mImages.push_back(BitMaps::si1_0);
  mImages.push_back(BitMaps::si1_1);
  mImages.push_back(BitMaps::si2_0);
  mImages.push_back(BitMaps::si2_1);
  mImages.push_back(BitMaps::zeta);
}

void MultiBladeGenerator::generateData() {
  constexpr size_t DATA_LENGTH = sizeof(VMM3Data);

  // We loop over all readout counts. For a given Fiber and FEN, we use two iterations to
  // generate a channel pair that corresponds to a non-background pixel for a bitmap
  // associated with a given VMM.
  const size_t N = NumberOfReadouts / 2;

  for (size_t Count = 0; Count < N; Count++) {
    // Get FEN and Fibers Ids + Tof
    const uint8_t FiberId = Fuzzer.randU8WithMask(MultiBladeSettings.NFibers, MultiBladeSettings.FiberMask);
    const uint8_t FENId   = Fuzzer.randU8WithMask(MultiBladeSettings.NFENs,   MultiBladeSettings.FENMask);
    const double TofMs    = mTofDist.getValue();

    // Get the VMM Id
    const u_int8_t VMM0 = Fuzzer.randU8WithMask(MultiBladeSettings.NVMMs, MultiBladeSettings.VMMMask);
    const bool isEven = VMM0 % 2 == 0;

    // Parameters for pixel coordinates
    const bool isFreia = (Settings.Type == Parser::FREIA);
    double XChannel{32};
    double YChannel{32};

    const double R = isFreia ? 0.9 : 1.0;
    const double Delta = isFreia ? 15 : 14;
    const double Sg = (FiberId % 2) ? 1 : -1;

    for (size_t i: {0, 1}) {
      // Get a VMM3Data struct pointer for the next Buffer write position
      VMM3Data * ReadoutData = getReadoutDataPtr(2 * Count + i);

      ReadoutData->FiberId = FiberId;
      ReadoutData->FENId   = FENId;

      // Tof or not
      if (MultiBladeSettings.Tof) {
        ReadoutData->TimeHigh = getPulseTimeHigh();
        ReadoutData->TimeLow = getPulseTimeLow() + static_cast<uint32_t>(TofMs * mTicksPerMs);
      }

      else {
        ReadoutData->TimeHigh = getReadoutTimeHigh();
        ReadoutData->TimeLow = getReadoutTimeLow();
      }

      // Misc
      ReadoutData->DataLength = DATA_LENGTH;
      ReadoutData->OTADC = 1000;
      assert(ReadoutData->DataLength == 20);

      // Get VMM Id
      const u_int8_t VMM = 2 * VMM0 + i;
      ReadoutData->VMM = VMM;
      Image image = mImages[VMM0 % mImages.size()];

      if (i == 0) {
        const int index = Fuzzer.randomInterval(0, image.size());
        const auto &[x, y] = image[index];

        if (isFreia) {
          XChannel = 63 - (16 + R * x + Sg * Delta);
          YChannel = 47 - y;
        }
        else {
          XChannel = 16 + x;
          YChannel = 63 - (16 + R * y + Sg * Delta);
        }
      }

      // Store channel
      ReadoutData->Channel = (i == 0) ? YChannel : XChannel;

      // Short time delta between correlated X- and Y-channels
      if (i == 0) {
        addTicksBtwReadoutsToReadoutTime();
      }

      // Large time delta between uncorrelated X- and Y-channels
      else {
        addTickBtwEventsToReadoutTime();
      }

      if (MultiBladeSettings.Debug) {
        if (i == 0) {
          fmt::print("Fiber = {}\n", FiberId);
          fmt::print("FENId = {}\n", FENId);
          fmt::print("VMM0  = {}\n", VMM0);
        }

        std::string circle = isEven ? "Upper" : "Lower";
        std::string axis = i == 0 ? "X" : "Y";
        std::string nl = i == 0 ? "\n" : "\n\n";

        fmt::print("{} {}: VMM = {}{}", circle, axis, VMM, nl);
      }
    }
  }
}

VMM3Data *MultiBladeGenerator::getReadoutDataPtr(size_t Index) {
  return (VMM3Data *) &Buffer[HeaderSize + Index * ReadoutDataSize];
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

} // namespace Freia

// GCOVR_EXCL_STOP
