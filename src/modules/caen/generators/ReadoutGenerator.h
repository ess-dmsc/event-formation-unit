// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial CAEN readouts
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <caen/readout/DataParser.h>
#include <generators/essudpgen/ReadoutGeneratorBase.h>
#include <generators/functiongenerators/DistributionGenerator.h>

namespace Caen {

class ReadoutGenerator : public ReadoutGeneratorBase {
public:
  // Settings local to CAEN data generator
  struct {
    std::string Detector;
    bool Tof{false};
    bool Loki{false}; // implies four amplitudes

    // Masks are used to restrict the generated data
    int FiberMask{0xffffff}; // Fibers 0 - 23
    int FENMask{0xfff}; // FENs 0 - 11
    int GroupMask{0xffff}; // Groups 0 - 14
  } CaenSettings;

  ReadoutGenerator();

  uint8_t get8(int MaxVal, int Mask); // crap function

  void setTypeByName(std::string Name) {
      Settings.TypeOverride = NameToType[Name];
      printf("Detector %s has type %u\n", Name.c_str(),
             Settings.TypeOverride);
  }

protected:
  void generateData() override;

  ///\brief may or may not generate a readout due to
  /// the detector mask, hence the bool return value
  bool getRandomReadout(DataParser::CaenReadout &DR);




  ///\brief For TOF distribution calculations
  DistributionGenerator TofDist{1000.0/14};
  float TicksPerMs{88552.0};

};
} // namespace Caen
// GCOVR_EXCL_STOP
