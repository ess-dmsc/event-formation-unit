// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial VMM3 readouts for multi-blade based setups
//===----------------------------------------------------------------------===//

// GCOVR_EXCL_START

#include <common/readout/vmm3/VMM3Parser.h>
#include <common/testutils/bitmaps/BitMaps.h>
#include <modules/freia/generators/MultiBladeGenerator.h>
// Geometry selector removed; generators now rely on config only.

#include <fmt/core.h>

#include <cmath>

using namespace ESSReadout;

namespace Freia {

MultiBladeGenerator::MultiBladeGenerator()
    : ReadoutGeneratorBase(DetectorType::FREIA) {
  // clang-format off

  // Options
  app.add_option("--detector", MultiBladeSettings.Detector, "Specify detector name (Freia, ESTIA, or TBLMB)");

  app.add_option("--fibervals", MultiBladeSettings.FiberVals, "Number of Fiber values to generate");
  app.add_option("--fibermask", MultiBladeSettings.FiberMask, "Mask out unused fibers");

  app.add_option("--fenvals", MultiBladeSettings.FENVals, "Number of FEN values to generate");
  app.add_option("--fenmask", MultiBladeSettings.FENMask, "Mask out unused FENs");

  app.add_option("--vmmvals", MultiBladeSettings.VMMVals, "Number of VMMs (Hybrids)");
  app.add_option("--vmmmask", MultiBladeSettings.VMMMask, "Mask out unused VMMs");

  // Flags

  // clang-format on

  // Load bitmaps
  mImages.push_back(&BitMaps::si1_0);
  mImages.push_back(&BitMaps::si1_1);
  mImages.push_back(&BitMaps::si2_0);
  mImages.push_back(&BitMaps::si2_1);
  mImages.push_back(&BitMaps::zeta);
}

void MultiBladeGenerator::generateData() {
  constexpr size_t DATA_LENGTH = sizeof(VMM3Data);

  // We loop over all readout counts. For a given Fiber and FEN, we use two
  // iterations to generate a channel pair that corresponds to a non-background
  // pixel for a bitmap associated with a given VMM.
  const size_t N = ReadoutsPerPacket / 2;

  for (size_t Count = 0; Count < N; Count++) {
    // Get FEN and Fibers Ids + Tof
    const uint8_t FiberId = Fuzzer.randU8WithMask(MultiBladeSettings.FiberVals,
                                                  MultiBladeSettings.FiberMask);
    const uint8_t FENId = Fuzzer.randU8WithMask(MultiBladeSettings.FENVals,
                                                MultiBladeSettings.FENMask);

    // Get the VMM Id
    const u_int8_t VMM0 = Fuzzer.randU8WithMask(MultiBladeSettings.VMMVals,
                                                MultiBladeSettings.VMMMask);
    const bool isEven = VMM0 % 2 == 0;

    // Parameters for pixel coordinates
    const bool isFreia = (Settings.Detector == DetectorType::FREIA);
    double XChannel{32};
    double YChannel{32};

    const double R = isFreia ? 0.9 : 1.0;
    const double Delta = isFreia ? 15 : 16;
    const double Sg = (FiberId % 2) ? 1 : -1;

    for (size_t i : {0, 1}) {
      // Get a VMM3Data struct pointer for the next Buffer write position
      VMM3Data *ReadoutData = getReadoutDataPtr(2 * Count + i);

      ReadoutData->FiberId = FiberId;
      ReadoutData->FENId = FENId;

      auto [timeHigh, timeLow] = generateReadoutTimeEveryN(2);
      ReadoutData->TimeHigh = timeHigh;
      ReadoutData->TimeLow = timeLow;

      // Misc
      ReadoutData->DataLength = DATA_LENGTH;
      ReadoutData->OTADC = 1000;
      assert(ReadoutData->DataLength == 20);

      // Get VMM Id
      const u_int8_t VMM = 2 * VMM0 + i;
      ReadoutData->VMM = VMM;
      const Image &image = *mImages[VMM0 % mImages.size()];

      if (i == 0) {
        const int index = Fuzzer.randomInterval(1, image.size() - 1);
        const auto &[x, y] = image[index];

        switch (Settings.Detector) {
        case DetectorType::FREIA:
          XChannel = 63 - (16 + R * x + Sg * Delta);
          YChannel = 47 - y;
          break;

        case DetectorType::ESTIA:
          XChannel = 16 + x;
          YChannel = 63 - (16 + R * y + Sg * Delta);
          break;

        case DetectorType::TBLMB:
          XChannel = 47 - y;
          YChannel = 63 - (16 + R * x + Sg * Delta);
          break;

        default:
          break;
        }
      }

      // Store channel
      ReadoutData->Channel = (i == 0) ? YChannel : XChannel;

      if (Settings.Debug) {
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
  return (VMM3Data *)&Buffer[HeaderSize + Index * ReadoutDataSize];
}

void MultiBladeGenerator::main() {
  // Call base class
  std::unique_ptr<FunctionGenerator> readoutTimeGenerator =
      std::make_unique<DistributionGenerator>(Settings.Frequency);
  ReadoutGeneratorBase::initialize(std::move(readoutTimeGenerator));

  // Set detector type
  Settings.Detector = MultiBladeSettings.Detector;
}

} // namespace Freia

// GCOVR_EXCL_STOP
