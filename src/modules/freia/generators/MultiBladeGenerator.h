// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial VMM3 readouts for multi-blade based setups
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <generators/essudpgen/ReadoutGeneratorBase.h>
#include <common/readout/vmm3/VMM3Parser.h>

namespace Freia {

using VMM3Data = ESSReadout::VMM3Parser::VMM3Data;

class MultiBladeGenerator : public ReadoutGeneratorBase {

public:
  //MultiBladeGenerator() : ReadoutGeneratorBase(ESSReadout::Parser::DetectorType::FREIA) {}
  MultiBladeGenerator();

  struct {
    std::string Detector;
    // bool Tof{false};
    // bool Loki{false}; // implies four amplitudes
    // bool Debug{false};

    // Masks are used to restrict the generated data
    int FiberVals{4};
    int FiberMask{0xff};   // Fibers 0 - 4

    int FENVals{4};
    int FENMask{0xff};     // FENs   0 - 4
  } MultiBladeSettings;

  protected:
  ///
  /// \brief Generate readout data and store these in the Buffer container
  ///
  void generateData() override;

  ///
  /// \brief For a given readout index, return a VMM3Data pointer to the associated Buffer
  ///        write position
  /// @param Index The readout index
  ///
  /// @return A pointer to
  ///
  VMM3Data *getReadoutDataPtr(size_t Index);

  const uint32_t mTimeToFirstReadout{1000};

  uint8_t randU8WithMask(int Range, int Mask);
};

} // namespace Freia

// GCOVR_EXCL_STOP
