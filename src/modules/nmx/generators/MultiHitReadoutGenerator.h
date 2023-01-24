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
#include <common/testutils/DataFuzzer.h>
#include <generators/essudpgen/ReadoutGeneratorBase.h>

namespace Nmx {
class MultiHitReadoutGenerator : public ReadoutGeneratorBase {
public:
  using ReadoutGeneratorBase::ReadoutGeneratorBase;

protected:
  void generateData() override;
  const uint32_t TimeToFirstReadout{1000};
  int64_t Number{0};
};
} // namespace Nmx

// GCOVR_EXCL_STOP
