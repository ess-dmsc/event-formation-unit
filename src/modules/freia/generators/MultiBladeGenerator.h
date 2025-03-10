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
#include <generators/functiongenerators/DistributionGenerator.h>
#include <common/readout/vmm3/VMM3Parser.h>

namespace Freia {

using VMM3Data = ESSReadout::VMM3Parser::VMM3Data;

class MultiBladeGenerator : public ReadoutGeneratorBase {

 public:
  MultiBladeGenerator();

  struct {
    std::string Detector{"Freia"};
    bool Tof{false};

    // Instruments
    bool Freia{false};
    bool Estia{false};
    bool Amor{false};

    bool Debug{false};

    // Masks are used to restrict the generated data
    int NFibers{4};      // Fibers 0 - 4
    int FiberMask{0xff};   

    int NFENs{2};        // FENs   0 - 2
    int FENMask{0xff};     

    int VMMMask{0xff};  
  } MultiBladeSettings;

  ///
  /// \brief Intercept the main function to access CLI parsed options
  void main();

 protected:
  ///
  /// \brief Generate readout data and store these in the Buffer container
  ///
  void generateData() override;

  ///
  /// \brief For a given readout index, return a VMM3Data pointer to the associated Buffer
  ///        write position
  ///
  /// @param Index The readout index
  ///
  /// @return A pointer to
  ///
  VMM3Data *getReadoutDataPtr(size_t Index);

  const uint32_t mTimeToFirstReadout{1000};

  size_t XDim;
  size_t YDim;

  ///\brief For TOF distribution calculations
  DistributionGenerator TofDist{1000.0/14};
  float TicksPerMs{88552.0};
};

} // namespace Freia

// GCOVR_EXCL_STOP
