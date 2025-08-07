// Copyright (C) 2022 - 2023 European Spallation Source, ERIC. See LICENSE file
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
#include <modules/nmx/generators/SmileReadoutGenerator.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Nmx {

void SmileReadoutGenerator::generateData() {
  auto DP = (uint8_t *)Buffer;
  DP += HeaderSize;

  uint16_t Panel = 0;
  float XLocal = 0;
  uint16_t YLocal = 0;
  uint8_t VMM = 0;
  uint16_t Channel = 0;
  uint8_t FEN = 0;
  std::map<uint8_t, uint8_t> XPanelToFEN{{0, 0}, {1, 1}, {2, 5}, {3, 4}};
  std::map<uint8_t, uint8_t> YPanelToFEN{{0, 7}, {1, 2}, {2, 6}, {3, 3}};

  for (uint32_t Readout = 0; Readout < ReadoutsPerPacket; Readout++) {
    auto ReadoutData = (ESSReadout::VMM3Parser::VMM3Data *)DP;

    ReadoutData->DataLength = sizeof(ESSReadout::VMM3Parser::VMM3Data);
    // NMX VMM readouts all have DataLength 20
    assert(ReadoutData->DataLength == 20);

    auto [readoutTimeHigh, readoutTimeLow] = generateReadoutTimeEveryN(2);
    ReadoutData->TimeHigh = readoutTimeHigh;
    ReadoutData->TimeLow = readoutTimeLow;

    ReadoutData->OTADC = 1000;
    ReadoutData->FiberId = 0;
    XTRACE(DATA, DEB, "Generating Readout %u", Readout);
    if ((Readout % 2) == 0) {
      Panel = rand() % 4;
      if (Panel <= 1) {
        XLocal = rand() % 160;
        YLocal = sqrt(6400 - pow(XLocal - 80, 2));
        XLocal += 260;
        YLocal += 260;
        YLocal = 640 - YLocal;
      } else if (Panel == 2) {
        XLocal = rand() % 640;
        YLocal = 640 - pow(25 - XLocal / 25, 2);
      } else {
        XLocal = rand() % 640;
        YLocal = 640 - pow(XLocal / 25, 2);
      }

      if (Panel <= 1) {
        Channel = 63 - int(XLocal) % 64;
        VMM = 9 - XLocal / 64;
      } else {
        Channel = int(XLocal) % 64;
        VMM = XLocal / 64;
      }
      FEN = XPanelToFEN[Panel];
      XTRACE(DATA, DEB,
             "Generating new coordinate, Panel: %u, XLocal: %u, YLocal: %u",
             Panel, XLocal, YLocal);
      XTRACE(DATA, DEB,
             "Generating readout for X, Channel: %u, VMM: %u, FEN: %u", Channel,
             VMM, FEN);
    } else {
      if ((Panel % 2) != 0) {
        Channel = 63 - YLocal % 64;
        VMM = 9 - YLocal / 64;
      } else {
        Channel = YLocal % 64;
        VMM = YLocal / 64;
      }
      FEN = YPanelToFEN[Panel];
      XTRACE(DATA, DEB,
             "Generating readout for Y, Channel: %u, VMM: %u, FEN: %u", Channel,
             VMM, FEN);
    }

    ReadoutData->VMM = VMM;
    ReadoutData->Channel = Channel;
    ReadoutData->FENId = FEN;
    DP += ReadoutDataSize;

    XTRACE(DATA, DEB,
           "Generating readout, FiberId: %u, FENId:%u, VMM:%u, Channel:%u, "
           "TimeHigh:%u, TimeLow:%u",
           ReadoutData->FiberId, ReadoutData->FENId, ReadoutData->VMM,
           ReadoutData->Channel, ReadoutData->TimeHigh, ReadoutData->TimeLow);
  }
}

} // namespace Nmx

// GCOVR_EXCL_STOP
