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

#include <cmath>

namespace Freia {

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
    ReadoutData->FiberId = (Count / 10) % Settings.NFibers;
    ReadoutData->FENId = 0x00;

    ReadoutData->DataLength = DATA_LENGTH;
    assert(ReadoutData->DataLength == 20);

    ReadoutData->TimeHigh = getReadoutTimeHigh();
    ReadoutData->TimeLow = getReadoutTimeLow();
    ReadoutData->VMM = Count & 0x3;
    ReadoutData->OTADC = 1000;

    if ((Count % 2) == 0) {
      Angle = Fuzzer.random8() * 360.0 / 255;
      XChannel = 44.0 - ReadoutData->FiberId + 10.0 * cos(Angle * DEG_TO_RADS);
      YChannel = 30.0 + 10.0 * sin(Angle * DEG_TO_RADS);
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
