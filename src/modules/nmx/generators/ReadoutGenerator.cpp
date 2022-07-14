// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial VMM3 readouts
// based on NMX ICD document
// https://project.esss.dk/owncloud/index.php/f/9513092
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <math.h>
#include <modules/nmx/generators/ReadoutGenerator.h>
#include <time.h>

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stdexcept>

void Nmx::ReadoutGenerator::generateData() {
  auto DP = (uint8_t *)Buffer;
  DP += HeaderSize;

  uint16_t Panel = 0;
  uint16_t XLocal = 0;
  uint16_t YLocal = 0;
  uint8_t VMM = 0;
  uint16_t Channel = 0;
  uint8_t FEN = 0;
  std::map<uint8_t, uint8_t> XPanelToFEN { {0, 0}, {1, 1}, {2, 5}, {3, 4}};
  std::map<uint8_t, uint8_t> YPanelToFEN { {0, 7}, {1, 2}, {2, 6}, {3, 3}};

  uint32_t TimeLow = TimeLowOffset + TimeToFirstReadout;
  for (uint32_t Readout = 0; Readout < Settings.NumReadouts; Readout++) {
    auto ReadoutData = (ESSReadout::VMM3Parser::VMM3Data *)DP;

    ReadoutData->DataLength = sizeof(ESSReadout::VMM3Parser::VMM3Data);
    // NMX VMM readouts all have DataLength 20
    assert(ReadoutData->DataLength == 20);

    ReadoutData->TimeHigh = TimeHigh;
    ReadoutData->TimeLow = TimeLow;
    ReadoutData->OTADC = 1000;

    Panel = (Readout / 1280)%4;
    XLocal = Readout % 160;
    YLocal = abs(XLocal - 150);
    
    
    if (Readout % 2){
      Channel = XLocal%64;
      VMM = XLocal/64;
      FEN = XPanelToFEN[Panel];
    }
    else{
      Channel = YLocal%64;
      VMM = YLocal/64;
      FEN = YPanelToFEN[Panel];
    }
    

    ReadoutData->VMM = VMM;
    ReadoutData->Channel = Channel;
    ReadoutData->FENId = FEN;
    DP += ReadoutDataSize;

    /// \todo work out why updating TimeLow is done this way, and if it applies
    /// to NMX
    if ((Readout % 2) == 0) {
      TimeLow += Settings.TicksBtwReadouts;
    } else {
      TimeLow += Settings.TicksBtwEvents;
    }
  }
}

// GCOVR_EXCL_STOP
