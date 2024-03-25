// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial DREAM readouts
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <algorithm>
#include <modules/dream/generators/ReadoutGenerator.h>

namespace Dream {

///\brief until ICD is reviewed this should be
/// considered completely wrong
/// Note to self RING is PHYSICAL RING!!!
void DreamReadoutGenerator::getRandomReadout(DataParser::DreamReadout &DR) {

  DR.DataLength = ReadoutDataSize;
  DR.TimeHigh = PulseTimeHigh;
  DR.TimeLow = PulseTimeLow;
  DR.OM = 0;
  DR.UnitId = 0;

  uint8_t DetectorSegment = Fuzzer.random8() % 5;
  // DetectorSegment = 3;
  switch (DetectorSegment) {
  case 0: { // BW EndCap
    uint8_t Sector = Fuzzer.random8() % 11;
    DR.FiberId = BWES6FiberId[Sector];
    DR.FENId = BWES6FENId[Sector];
    DR.Anode = std::min(Fuzzer.random8(), (uint8_t)63);
    DR.Cathode = std::min(Fuzzer.random8(), (uint8_t)95); /// cathodes == strips
    DR.UnitId = 6;                                        // SUMO6
  } break;

  case 1: { // FW EndCap
    uint8_t Sector = Fuzzer.random8() % 5;
    DR.FiberId = FWES6FiberId[Sector];
    DR.FENId = FWES6FENId[Sector];
    DR.Anode = std::min(Fuzzer.random8(), (uint8_t)63);   /// anodes == wires
    DR.Cathode = std::min(Fuzzer.random8(), (uint8_t)95); /// cathodes == strips
    DR.UnitId = 6;                                        // SUMO6
  } break;

  case 2: { // Mantle
    uint8_t Sector = Fuzzer.random8() % 30;
    DR.FiberId = MNTLFiberId[Sector];
    DR.FENId = MNTLFENId[Sector];
    DR.Anode = std::min(Fuzzer.random8(), (uint8_t)127);
    DR.Cathode = Fuzzer.random8();
  } break;

  case 3: { // HR
    uint8_t Sector = Fuzzer.random8() % 17;
    uint8_t Instance = Fuzzer.random8() % 2;
    DR.FiberId = HRFiberId[Sector];
    DR.FENId = HRFENId[Sector];
    DR.Anode = (Fuzzer.random8() & 0x7) * 32;
    DR.Cathode = Fuzzer.random8() & 0x3f;
    DR.UnitId = Instance;
  } break;

  case 4: { // SANS
    uint8_t Sector = Fuzzer.random8() % 18;
    uint8_t Instance = Fuzzer.random8() % 2;
    DR.FiberId = SANSFiberId[Sector];
    DR.FENId = SANSFENId[Sector];
    DR.Anode = Fuzzer.random8();
    DR.Cathode = Fuzzer.random8() & 0x3f;
    DR.UnitId = Instance;
  } break;
  }
  // DR.FiberId = 2; // debug
  // DR.FENId = 0; // debug
  // printf("Detector %u, Ring %u, FEN %u\n", DetectorSegment, DR.FiberId,
  // DR.FENId);
}

/// \brief implementation of virtual functio from base class
void DreamReadoutGenerator::generateData() {
  auto DP = (uint8_t *)Buffer;
  DP += HeaderSize;

  uint32_t Readouts{0};
  DataParser::DreamReadout DR;

  while (Readouts < Settings.NumReadouts) {
    getRandomReadout(DR);
    memcpy(DP, &DR, ReadoutDataSize);
    DP += ReadoutDataSize;

    // All readouts are events for DREAM
    PulseTimeLow += Settings.TicksBtwEvents;

    if (PulseTimeLow >= 88052499) {
      PulseTimeLow -= 88052499;
      PulseTimeHigh += 1;
    }
    Readouts++;
  }
}
} // namespace Dream
// GCOVR_EXCL_STOP
