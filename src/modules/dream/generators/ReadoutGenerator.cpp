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

///\brief ICD has been reviewed, but it would be surprising
// if this was 100% correct
bool DreamReadoutGenerator::getRandomReadout(DataParser::DreamReadout &DR) {
  DR.DataLength = ReadoutDataSize;
  DR.TimeHigh = PulseTimeHigh;
  DR.TimeLow = PulseTimeLow;
  DR.OM = 0;
  DR.UnitId = 0; //will be determined later

  uint8_t DetectorMask = 1 << (Fuzzer.random8() % 5);

  switch (DetectorMask) {
    case 1: { // BW EndCap
      if (not (DetectorMask & Settings.FreeParam1)){
        return false;
      }
      uint8_t Sector = Fuzzer.random8() % 11;
      DR.UnitId = 6;                                        // SUMO6
      if (Settings.FreeParam2 != -1) {
          Sector = Settings.FreeParam2;
      }
      if (Settings.FreeParam3 != -1) {
          DR.UnitId = Settings.FreeParam3;
      }

      DR.FiberId = BWES6FiberId[Sector];
      DR.FENId = BWES6FENId[Sector];
      DR.Anode = std::min(Fuzzer.random8(), (uint8_t)63);
      DR.Cathode = std::min(Fuzzer.random8(), (uint8_t)95); /// cathodes == strips

    } break;

    case 2: { // FW EndCap
      if (not (DetectorMask & Settings.FreeParam1)){
        return false;
      }
      uint8_t Sector = Fuzzer.random8() % 5;
      DR.UnitId = 6;
      if (Settings.FreeParam2 != -1) {
          Sector = Settings.FreeParam2 %5;
      }
      if (Settings.FreeParam3 != -1) {
          DR.UnitId = Settings.FreeParam3;
      }
      DR.FiberId = FWES6FiberId[Sector];
      DR.FENId = FWES6FENId[Sector];
      DR.Anode = std::min(Fuzzer.random8(), (uint8_t)63);   /// anodes == wires
      DR.Cathode = std::min(Fuzzer.random8(), (uint8_t)95); /// cathodes == strips
    } break;

    case 4: { // Mantle
      if (not (DetectorMask & Settings.FreeParam1)){
        return false;
      }
      uint8_t Sector = Fuzzer.random8() % 30;
      DR.FiberId = MNTLFiberId[Sector];
      DR.FENId = MNTLFENId[Sector];
      DR.Anode = Fuzzer.random8() & 0x7f;
      DR.Cathode = Fuzzer.random8();
    } break;

    case 8: { // HR
      if (not (DetectorMask & Settings.FreeParam1)){
        return false;
      }
      //uint8_t Sector = Fuzzer.random8() % 17;
      uint8_t Sector = Fuzzer.random8() % 17;
      uint8_t Instance = Fuzzer.random8() % 2;
      DR.FiberId = HRFiberId[Sector];
      DR.FENId = HRFENId[Sector];
      DR.Anode = std::min(Fuzzer.random8(), (uint8_t)190);
      DR.Cathode = Fuzzer.random8() & 0x3f;
      DR.UnitId = Instance;
    } break;

    case 16: { // SANS
      if (not (DetectorMask & Settings.FreeParam1)){
        return false;
      }
      uint8_t Sector = Fuzzer.random8() % 18;
      uint8_t Instance = Fuzzer.random8() % 2;
      DR.FiberId = SANSFiberId[Sector];
      DR.FENId = SANSFENId[Sector];
      DR.Anode = std::min(Fuzzer.random8(), (uint8_t)190);
      DR.Cathode = Fuzzer.random8() & 0x3f;
      DR.UnitId = Instance;
    } break;
  }
  return true;
}

/// \brief implementation of virtual functio from base class
void DreamReadoutGenerator::generateData() {
  auto DP = (uint8_t *)Buffer;
  DP += HeaderSize;

  uint32_t Readouts{0};
  DataParser::DreamReadout DR;

  while (Readouts < Settings.NumReadouts) {
    bool Valid = getRandomReadout(DR);
    if (not Valid) {
      continue;
    }
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
