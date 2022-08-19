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

#include <common/debug/Trace.h>
#include <math.h>
#include <modules/nmx/generators/TrackReadoutGenerator.h>
#include <time.h>

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stdexcept>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

void Nmx::TrackReadoutGenerator::generateData() {
  auto DP = (uint8_t *)Buffer;
  DP += HeaderSize;

  uint16_t Panel = 0;
  uint16_t XLocal = 0;
  uint16_t YLocal = 0;
  uint16_t FinalXLocal = 0;
  uint16_t FinalYLocal = 0;
  uint8_t VMM = 0;
  uint16_t Channel = 0;
  uint8_t FEN = 0;
  float XDiff = 0.0;
  float YDiff = 0.0;
  std::map<uint8_t, uint8_t> XPanelToFEN{{0, 0}, {1, 1}, {2, 5}, {3, 4}};
  std::map<uint8_t, uint8_t> YPanelToFEN{{0, 7}, {1, 2}, {2, 6}, {3, 3}};

  uint32_t TimeLow = TimeLowOffset + TimeToFirstReadout;
  for (uint32_t Readout = 0; Readout < Settings.NumReadouts; Readout++) {
    auto ReadoutData = (ESSReadout::VMM3Parser::VMM3Data *)DP;

    ReadoutData->DataLength = sizeof(ESSReadout::VMM3Parser::VMM3Data);
    // NMX VMM readouts all have DataLength 20
    assert(ReadoutData->DataLength == 20);

    ReadoutData->TimeHigh = TimeHigh;
    ReadoutData->TimeLow = TimeLow;
    ReadoutData->OTADC = 1000;
    ReadoutData->RingId = 0;
    XTRACE(DATA, DEB, "Generating Readout %u", Readout);
    if ((Readout % ReadoutsPerEvent) == 0) {
      Panel = rand() % 4;
      FinalXLocal = rand() % 640;
      FinalYLocal = 639 - (abs(1.2 * (FinalXLocal - 128)));
      XDiff = (((float)rand()/(float)(RAND_MAX)) * 2)-1;
      YDiff = (((float)rand()/(float)(RAND_MAX)) * 2)-1;
       XTRACE(DATA, DEB,
              "Generating new coordinate, Panel: %u, XLocal: %u, YLocal: %u",
              Panel, FinalXLocal, FinalYLocal);
    }
      
    if ((Readout % 2) == 0) { 
      XLocal = FinalXLocal + XDiff * ((ReadoutsPerEvent - (Readout % ReadoutsPerEvent))/2);
      if (Panel <= 1) {
        Channel = 63 - XLocal % 64;
        VMM = 9 - XLocal / 64;
      } else {
        Channel = XLocal % 64;
        VMM = XLocal / 64;
      }
      FEN = XPanelToFEN[Panel];
     
      XTRACE(DATA, DEB,
              "Generating readout for X Coord: %u, Channel: %u, VMM: %u, FEN: %u", XLocal, Channel,
              VMM, FEN);
    } else {
      YLocal = FinalYLocal + YDiff * ((ReadoutsPerEvent - (Readout % ReadoutsPerEvent))/2);
      if ((Panel % 2) != 0) {
        Channel = 63 - YLocal % 64;
        VMM = 9 - YLocal / 64;
      } else {
        Channel = YLocal % 64;
        VMM = YLocal / 64;
      }
      FEN = YPanelToFEN[Panel];
      XTRACE(DATA, DEB,
             "Generating readout for Y Coord: %u, Channel: %u, VMM: %u, FEN: %u",YLocal, Channel,
             VMM, FEN);
    }

    ReadoutData->VMM = VMM;
    ReadoutData->Channel = Channel;
    ReadoutData->FENId = FEN;
    DP += ReadoutDataSize;

    /// \todo work out why updating TimeLow is done this way, and if it applies
    /// to NMX
    if (((Readout + 1) % ReadoutsPerEvent) != 0) {
      TimeLow += Settings.TicksBtwReadouts;
    } else {
      TimeLow += Settings.TicksBtwEvents;
    }
    if (TimeLow >= 88052499) {
      TimeLow -= 88052499;
      TimeHigh += 1;
    }
    XTRACE(DATA, DEB,
           "Generating readout, RingId: %u, FENId:%u, VMM:%u, Channel:%u, "
           "TimeHigh:%u, TimeLow:%u",
           ReadoutData->RingId, ReadoutData->FENId, ReadoutData->VMM,
           ReadoutData->Channel, ReadoutData->TimeHigh, ReadoutData->TimeLow);
  }
}

// GCOVR_EXCL_STOP
