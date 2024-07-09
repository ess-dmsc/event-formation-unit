// Copyright (C) 2023 - 2024 European Spallation Source, ERIC. See LICENSE file
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
  app.add_flag("--tof", DreamSettings.Tof,
                "generate tof distribution");
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
bool ReadoutGenerator::getRandomReadout(DataParser::CDTReadout &ReadoutData) {
  ReadoutData.DataLength = ReadoutDataSize;

  if (DreamSettings.Tof) {
    double TofMs = TofDist.getValue();
    ReadoutData.TimeHigh = pulseTime.getTimeHigh();
    ReadoutData.TimeLow = pulseTime.getTimeLow() + TofMs * TicksPerMs;
  } else {
    ReadoutData.TimeHigh = getReadoutTimeHigh();
    ReadoutData.TimeLow = getReadoutTimeLow();
  }
  ReadoutData.OM = 0;
  ReadoutData.UnitId = 0; //will be determined later

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
      ReadoutData.UnitId = 6; // SUMO6
      if (DreamSettings.Param3 != -1) {
          ReadoutData.UnitId = DreamSettings.Param3;
      }

      ReadoutData.FiberId = BWES6FiberId[Sector];
      ReadoutData.FENId = BWES6FENId[Sector];
      ReadoutData.Anode = std::min(Fuzzer.random8(), (uint8_t)63);
      ReadoutData.Cathode = std::min(Fuzzer.random8(), (uint8_t)95); /// cathodes == strips

    } break;

    case FwEndCap: { // FW EndCap
      uint8_t Sector = Fuzzer.random8() % 5;
      ReadoutData.UnitId = 6;
      if (DreamSettings.Param2 != -1) {
          Sector = DreamSettings.Param2 %5;
      }
      if (DreamSettings.Param3 != -1) {
          ReadoutData.UnitId = DreamSettings.Param3;
      }
      ReadoutData.FiberId = FWES6FiberId[Sector];
      ReadoutData.FENId = FWES6FENId[Sector];
      ReadoutData.Anode = std::min(Fuzzer.random8(), (uint8_t)63);   /// anodes == wires
      ReadoutData.Cathode = std::min(Fuzzer.random8(), (uint8_t)95); /// cathodes == strips
    } break;

    case Mantle: { // Mantle
      uint8_t Sector = Fuzzer.random8() % 30;
      ReadoutData.FiberId = MNTLFiberId[Sector];
      ReadoutData.FENId = MNTLFENId[Sector];
      ReadoutData.Anode = Fuzzer.random8() & 0x7f;
      ReadoutData.Cathode = Fuzzer.random8();
    } break;

    case HR: { // HR
      //uint8_t Sector = Fuzzer.random8() % 17;
      uint8_t Sector = Fuzzer.random8() % 17;
      uint8_t Instance = Fuzzer.random8() % 2;
      ReadoutData.FiberId = HRFiberId[Sector];
      ReadoutData.FENId = HRFENId[Sector];
      ReadoutData.Anode = std::min(Fuzzer.random8(), (uint8_t)190);
      ReadoutData.Cathode = Fuzzer.random8() & 0x3f;
      ReadoutData.UnitId = Instance;
    } break;

    case SANS: { // SANS
      uint8_t Sector = Fuzzer.random8() % 18;
      uint8_t Instance = Fuzzer.random8() % 2;
      ReadoutData.FiberId = SANSFiberId[Sector];
      ReadoutData.FENId = SANSFENId[Sector];
      ReadoutData.Anode = std::min(Fuzzer.random8(), (uint8_t)190);
      ReadoutData.Cathode = Fuzzer.random8() & 0x3f;
      ReadoutData.UnitId = Instance;
    } break;
  }
  return true;
}

/// \brief implementation of virtual functio from base class
void ReadoutGenerator::generateData() {
  auto DataPtr = (uint8_t *)Buffer;
  DataPtr += HeaderSize;

  uint32_t Readouts{0};
  DataParser::CDTReadout ReadoutData;

  while (Readouts < Settings.NumReadouts) {
    bool Valid = getRandomReadout(ReadoutData);
    if (not Valid) {
      continue;
    }
    memcpy(DataPtr, &ReadoutData, ReadoutDataSize);
    DataPtr += ReadoutDataSize;

    // Increment the time for next readout
    addTickBtwEventsToReadoutTime();
    Readouts++;
  }
}

} // namespace Dream
// GCOVR_EXCL_STOP
