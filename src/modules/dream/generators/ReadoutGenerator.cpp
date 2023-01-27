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


void DreamReadoutGenerator::getRandomReadout(
      DataParser::DreamReadout & DR) {

  uint8_t Sector = Fuzzer.random8()%11;

  // // For now create readouts for SUMO6
  DR.DataLength = ReadoutDataSize;
  DR.TimeHigh = TimeHigh;
  DR.TimeLow = TimeLow;

  DR.RingId = S6RingId[Sector];
  DR.FENId = S6FENId[Sector];
  DR.DataLength = ReadoutDataSize;

  DR.OM = 0;
  DR.Unused = 0;
  DR.Anode = Fuzzer.random8();; ///\todo wires - still dont fully understand sumo geometry
  DR.Cathode = Fuzzer.random8(); ///\todo cathodes are strips - same as above
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
