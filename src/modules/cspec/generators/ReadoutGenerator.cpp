// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial VMM3 readouts
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <math.h>
#include <time.h>
#include <modules/cspec/generators/ReadoutGenerator.h>
#include <stdexcept>



void Cspec::ReadoutGenerator::generateData(uint16_t NumReadouts) {
  auto DP = (uint8_t *)Buffer;
  DP += HeaderSize;

  uint16_t XGlobal = 0;
  uint16_t YGlobal = 0;
  uint8_t VMM = 0;
  uint16_t Channel = 0;


  uint32_t TimeLow = TimeLowOffset + TimeToFirstReadout;
  for (auto Readout = 0; Readout < NumReadouts; Readout++) {
    auto ReadoutData = (ESSReadout::VMM3Parser::VMM3Data *)DP;

    ReadoutData->DataLength = sizeof(ESSReadout::VMM3Parser::VMM3Data);
    assert(ReadoutData->DataLength == 20);

    ReadoutData->TimeHigh = TimeHigh;
    ReadoutData->TimeLow = TimeLow;
    ReadoutData->OTADC = 1000;

    if ((Readout % 16) == 0){
      XGlobal = Fuzzer.random8() * 12 / 255;
    } 
    YGlobal = 12 * abs(XGlobal-2) + 140 * Readout % 16;


    ReadoutData->RingId = 5;
    ReadoutData->FENId = 1 + (Readout % 2);

    // Wire X and Z direction
    if ((Readout % 2) == 0) {
      if (XGlobal < 2) {
        VMM = 0;
        Channel = (XGlobal * 16) + 32 + Fuzzer.random8() * 16 / 255;
      }
      else{
        VMM = 1;
        Channel = (XGlobal - 2) * 16 + Fuzzer.random8() * 16 / 255;
      }
    }
    // Grid Y direction
    else {
      if (YGlobal < 6){
        VMM = 5;
        Channel = 5 - YGlobal;
      }
      else if (YGlobal < 70){
        VMM = 4;
        Channel = 69 - YGlobal;
      }
      else if (YGlobal < 134){
        VMM = 3;
        Channel = 133 - YGlobal;
      }
      else{
        VMM = 2;
        Channel = 139 - YGlobal;
      }
    }

    ReadoutData->VMM = VMM;
    ReadoutData->Channel = Channel;

    DP += VMM3DataSize;
    if ((Readout % 3) == 0) {
      TimeLow += TimeBtwReadout;
    } else {
      TimeLow += TimeBtwEvents;
    }
  }
}




// GCOVR_EXCL_STOP