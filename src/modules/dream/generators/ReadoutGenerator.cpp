// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial DREAM readouts
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <modules/dream/generators/ReadoutGenerator.h>

namespace Dream {

ReadoutGenerator::ReadoutGenerator() : ReadoutGeneratorBase(ESSReadout::Parser::DetectorType::DREAM) {
  app.add_option("--p1", DreamSettings.DetectorMask,
                "Detector element mask");
  app.add_option("--p2", DreamSettings.Param2,
                "Free parameter for DREAM datagenerator");
  app.add_option("--p3", DreamSettings.Param3,
                "Free parameter for DREAM datagenerator");
}

enum Detector {BwEndCap = 1, FwEndCap = 2, Mantle = 4, HR = 8, SANS = 16};

///\brief ICD has been reviewed, but it would be surprising
// if this was 100% correct
bool ReadoutGenerator::getRandomReadout(DataParser::DreamReadout &DR) {
  DR.DataLength = ReadoutDataSize;
  DR.TimeHigh = getReadoutTimeHigh();
  DR.TimeLow = getReadoutTimeLow();
  DR.OM = 0;
  DR.UnitId = 0; //will be determined later

  // Each of the five detector elements has its own 'bit'
  // this is used for masking. If the selected value is
  // not in the mask we do not generate anything and return false
  uint8_t DetectorValue = 1 << (Fuzzer.random8() % 5);
  if (not (DetectorValue & DreamSettings.DetectorMask)){
    return false;
  }

  switch (DetectorValue) {
    case BwEndCap: {
      uint8_t Sector = Fuzzer.random8() % 11;

      if (DreamSettings.Param2 != -1) {
          Sector = DreamSettings.Param2;
      }
      DR.UnitId = 6; // SUMO6
      if (DreamSettings.Param3 != -1) {
          DR.UnitId = DreamSettings.Param3;
      }

      DR.FiberId = BWES6FiberId[Sector];
      DR.FENId = BWES6FENId[Sector];
      DR.Anode = std::min(Fuzzer.random8(), (uint8_t)63);
      DR.Cathode = std::min(Fuzzer.random8(), (uint8_t)95); /// cathodes == strips

    } break;

    case FwEndCap: { // FW EndCap
      uint8_t Sector = Fuzzer.random8() % 5;
      DR.UnitId = 6;
      if (DreamSettings.Param2 != -1) {
          Sector = DreamSettings.Param2 %5;
      }
      if (DreamSettings.Param3 != -1) {
          DR.UnitId = DreamSettings.Param3;
      }
      DR.FiberId = FWES6FiberId[Sector];
      DR.FENId = FWES6FENId[Sector];
      DR.Anode = std::min(Fuzzer.random8(), (uint8_t)63);   /// anodes == wires
      DR.Cathode = std::min(Fuzzer.random8(), (uint8_t)95); /// cathodes == strips
    } break;

    case Mantle: { // Mantle
      uint8_t Sector = Fuzzer.random8() % 30;
      DR.FiberId = MNTLFiberId[Sector];
      DR.FENId = MNTLFENId[Sector];
      DR.Anode = Fuzzer.random8() & 0x7f;
      DR.Cathode = Fuzzer.random8();
    } break;

    case HR: { // HR
      //uint8_t Sector = Fuzzer.random8() % 17;
      uint8_t Sector = Fuzzer.random8() % 17;
      uint8_t Instance = Fuzzer.random8() % 2;
      DR.FiberId = HRFiberId[Sector];
      DR.FENId = HRFENId[Sector];
      DR.Anode = std::min(Fuzzer.random8(), (uint8_t)190);
      DR.Cathode = Fuzzer.random8() & 0x3f;
      DR.UnitId = Instance;
    } break;

    case SANS: { // SANS
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
void ReadoutGenerator::generateData() {
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

    // Increment the time for next readout
    addTickBtwEventsToReadoutTime();

    Readouts++;
  }
}

} // namespace Dream
// GCOVR_EXCL_STOP
