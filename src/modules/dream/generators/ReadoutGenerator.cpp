// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial DREAM readouts
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <common/debug/Trace.h>
#include <modules/dream/generators/ReadoutGenerator.h>
#include <string.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Dream {

///\brief until ICD is reviewed this should be
/// considered completely wrong
/// Note to self RING is PHYSICAL RING!!!
void DreamReadoutGenerator::getRandomReadout(
      DataParser::DreamReadout & DR) {

  DR.DataLength = ReadoutDataSize;
  DR.TimeHigh = TimeHigh;
  DR.TimeLow = TimeLow;
  DR.OM = 0;
  DR.Unused = 0;

  uint8_t DetectorSegment = Fuzzer.random8()%5;
  switch (DetectorSegment) {
    case 0: { // BW EndCap
      uint8_t Sector = Fuzzer.random8()%11;
      DR.RingId = BWES6RingId[Sector];
      DR.FENId = BWES6FENId[Sector];
      DR.Anode = Fuzzer.random8(); /// anodes == wires
      DR.Cathode = Fuzzer.random8(); /// cathodes == strips
      }
    break;

    case 1: { // FW EndCap
      uint8_t Sector = Fuzzer.random8()%5;
      DR.RingId = FWES6RingId[Sector];
      DR.FENId = FWES6FENId[Sector];
      DR.Anode = Fuzzer.random8(); /// anodes == wires
      DR.Cathode = Fuzzer.random8(); /// cathodes == strips
      }
    break;

    case 2: { // Mantle
      uint8_t Sector = Fuzzer.random8()%30;
      DR.RingId = MNTLRingId[Sector];
      DR.FENId = MNTLFENId[Sector];
      DR.Anode = Fuzzer.random8();
      DR.Cathode = Fuzzer.random8();
      }
    break;

    case 3: { // HR
      uint8_t Sector = Fuzzer.random8()%17;
      DR.RingId = HRRingId[Sector];
      DR.FENId =  HRFENId[Sector];
      DR.Anode = 0;
      DR.Cathode = 0;
      }
    break;

    case 4: { // SANS
      uint8_t Sector = Fuzzer.random8()%17;
      DR.RingId = SANSRingId[Sector];
      DR.FENId =  SANSFENId[Sector];
      DR.Anode = 0;
      DR.Cathode = 0;
      }
    break;
  }
  // DR.RingId = 2; // debug
  // DR.FENId = 0; // debug
  // printf("Detector %u, Ring %u, FEN %u\n", DetectorSegment, DR.RingId, DR.FENId);
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
    TimeLow += Settings.TicksBtwEvents;

    if (TimeLow >= 88052499) {
      TimeLow -= 88052499;
      TimeHigh += 1;
    }
    Readouts++;
  }
}
} // namespace Dream
// GCOVR_EXCL_STOP
