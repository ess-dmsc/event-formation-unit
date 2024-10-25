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


  ///\brief Generate a random integer
  ///\param Values number of values generated (from 0 to Values -1), must be
  /// nonzero and <= 32
  ///\param Mask the (32 bit) bitmask of allowed values, must be nonzero
  ///
  /// Constraints:
  /// 1) The number must belong to the interval 0 to (Values - 1)
  /// 2) The number, represented as a bit, must be accepted by the supplied mask
  ///
  /// Ex: If Mask is 0x03 (11 in binary), the only allowed values are 0
  /// and 1 corresponding to the 0'th and first bit.
  ///
  /// Ex: Mask 0x09 would allow values 0 and 3, so randU8WithMask(16, 0x09)
  /// will return a random sequence from the set (0, 3) where as
  /// randU8WithMask(8, 0x09) will only return 0's.
  uint8_t randU8WithMask(int Values, int Mask);


  ///\brief For TOF distribution calculations
  DistributionGenerator TofDist{1000.0/14};
  float TicksPerMs{88552.0};

};
} // namespace Caen
// GCOVR_EXCL_STOP
