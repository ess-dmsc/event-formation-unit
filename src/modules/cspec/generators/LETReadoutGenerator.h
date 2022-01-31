// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial VMM3a readouts with variable number
/// of readouts
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <common/readout/vmm3/VMM3Parser.h>
#include <common/readout/vmm3/generators/ReadoutGeneratorBase.h>
#include <common/testutils/DataFuzzer.h>

namespace Cspec {
class LETReadoutGenerator : public ReadoutGeneratorBase {
public:
  using ReadoutGeneratorBase::ReadoutGeneratorBase;

protected:
  void generateData() override;
  const uint32_t TimeToFirstReadout{1000};
};
} // namespace Cspec
// GCOVR_EXCL_STOP
