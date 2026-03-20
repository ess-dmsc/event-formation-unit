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

namespace trex {

class LETReadoutGenerator : public ReadoutGeneratorBase {
public:
  LETReadoutGenerator(): ReadoutGeneratorBase(DetectorType::TREX) {}

protected:
  void generateData() override;

  uint64_t GlobalReadout{0};
};

} // namespace trex
// GCOVR_EXCL_STOP
