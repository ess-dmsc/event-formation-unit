// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial VMM3 readouts
// based on CSPEC ICD document
// https://project.esss.dk/owncloud/index.php/f/14482406
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <math.h>
#include <modules/cspec/generators/ReadoutGenerator.h>
#include <time.h>

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stdexcept>


void Cspec::ReadoutGenerator::generateData() {
  auto DP = (uint8_t *)Buffer;
  DP += HeaderSize;

  uint16_t XGlobal = 0;
  uint16_t XLocal = 0;
  uint16_t YLocal = 0;
  uint8_t VMM = 0;
  uint16_t Channel = 0;

  uint32_t TimeLow = TimeLowOffset + TimeToFirstReadout;
  for (uint32_t Readout = 0; Readout < Settings.NumReadouts; Readout++) {
    auto ReadoutData = (ESSReadout::VMM3Parser::VMM3Data *)DP;

    ReadoutData->DataLength = sizeof(ESSReadout::VMM3Parser::VMM3Data);
    // CSPEC VMM readouts all have DataLength 20
    assert(ReadoutData->DataLength == 20);

    ReadoutData->TimeHigh = TimeHigh;
    ReadoutData->TimeLow = TimeLow;
    ReadoutData->OTADC = 1000;

    // CSPEC is 16 wires deep in Z direction
    // X is selected as a number between 0 and 11 as there are
    // 2 columns of 6 wires in the LET setup
    if ((Readout % 16) == 0) {
      XGlobal = Fuzzer.random8() * 12 / 255;
    }
    // Forms a tick shape, and stretches it into a taller rectangle
    // as for LET MaxX = 11 and MaxY = 50
    YLocal = 4 * abs(XGlobal - 2);

    // Readout generated for LET test, with Ring 5 and FENId 0 or 1
    ReadoutData->RingId = 5;
    ReadoutData->FENId = 0 + (Readout % 2);

    // Each column is 6 wires wide
    // Select the FEN based on whether XGlobal is in column 0 or column 1
    // Initialise XLocal as the local X value within each column
    if (XGlobal < 6) {
      ReadoutData->FENId = 0;
      XLocal = XGlobal;
    } else {
      ReadoutData->FENId = 1;
      XLocal = XGlobal - 6;
    }

    // Wire X and Z direction
    /// \todo check maths for calculating Channel is correct
    // All channel calculations are based on ICD linked at top of file
    if ((Readout % 2) == 0) {
      if (XLocal < 2) {
        VMM = 0;
        Channel = (XLocal * 16) + 32 + Fuzzer.random8() * 16 / 255;
      } else {
        VMM = 1;
        Channel = (XLocal - 2) * 16 + Fuzzer.random8() * 16 / 255;
      }
    }
    // Grid Y direction
    // Mappings of Y coordinates to channels and VMMs is complicated
    // Details are in ICD, with useful figure
    else {
      if (YLocal < 6) {
        VMM = 5;
        Channel = 5 - YLocal;
      } else if (YLocal < 70) {
        VMM = 4;
        Channel = 69 - YLocal;
      } else if (YLocal < 134) {
        VMM = 3;
        Channel = 133 - YLocal;
      } else {
        VMM = 2;
        Channel = 139 - YLocal;
      }
    }

    ReadoutData->VMM = VMM;
    ReadoutData->Channel = Channel;

    DP += VMM3DataSize;

    /// \todo work out why updating TimeLow is done this way, and if it applies to CSPEC
    if ((Readout % 2) == 0) {
      TimeLow += Settings.TicksBtwReadouts;
    } else {
      TimeLow += Settings.TicksBtwEvents;
    }
  }
}

// GCOVR_EXCL_STOP
