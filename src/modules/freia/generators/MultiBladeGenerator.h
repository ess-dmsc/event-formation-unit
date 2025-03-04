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
  MultiBladeGenerator() : ReadoutGeneratorBase(ESSReadout::Parser::DetectorType::FREIA) {}

protected:
  ///
  /// \brief Generate readout data and store these in Buffer container
  ///
  void generateData() override;

  ///
  /// \brief For a given readout index, return a VMM3Data pointer for the associated Buffer
  ///        write position
  /// @param Index The readout index
  ///
  /// @return A pointer to
  ///
  VMM3Data *getReadoutDataPtr(size_t Index);

  const uint32_t TimeToFirstReadout{1000};
};

} // namespace Freia

// GCOVR_EXCL_STOP
