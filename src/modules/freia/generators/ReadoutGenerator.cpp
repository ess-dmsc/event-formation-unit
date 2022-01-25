// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial VMM3 readouts
// based on Freia ICD document https://project.esss.dk/owncloud/index.php/f/14683667
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <math.h>
#include <time.h>
#include <modules/freia/generators/ReadoutGenerator.h>
#include <stdexcept>


void Freia::ReadoutGenerator::generateData() {
  auto DP = (uint8_t *)Buffer;
  DP += HeaderSize;

  double Angle = 0;
  double XChannel = 32;
  double YChannel = 32;

  uint32_t TimeLow = TimeLowOffset + TimeToFirstReadout;
  for (uint32_t Readout = 0; Readout < Settings.NumReadouts; Readout++) {
    auto ReadoutData = (ESSReadout::VMM3Parser::VMM3Data *)DP;
    ReadoutData->RingId = (Readout / 10) % Settings.NRings;
    ReadoutData->FENId = 0x00;
    ReadoutData->DataLength = sizeof(ESSReadout::VMM3Parser::VMM3Data);
    assert(ReadoutData->DataLength == 20);

    ReadoutData->TimeHigh = TimeHigh;
    ReadoutData->TimeLow = TimeLow;
    ReadoutData->VMM = Readout & 0x3;
    ReadoutData->OTADC = 1000;

    if ((Readout % 2) == 0) {
      Angle = Fuzzer.random8() * 360.0/ 255;
      XChannel = 44.0 - ReadoutData->RingId + 10.0 * cos(Angle * 2 * 3.14156 / 360.0);
      YChannel = 30.0 + 10.0 * sin(Angle * 2 * 3.14156 / 360.0);
    }

    if ((Readout % 2) == 0) {
      ReadoutData->Channel = YChannel;
    } else {
      ReadoutData->Channel = XChannel;
    }
    DP += VMM3DataSize;
    if ((Readout % 2) == 0) {
      TimeLow += Settings.TicksBtwReadouts;
    } else {
      TimeLow += Settings.TicksBtwEvents;
    }
  }
}




// GCOVR_EXCL_STOP
