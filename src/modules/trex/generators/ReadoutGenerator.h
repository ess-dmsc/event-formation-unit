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

namespace Trex {

class ReadoutGenerator : public ReadoutGeneratorBase {
public:
  ReadoutGenerator() : ReadoutGeneratorBase(DetectorType::TREX) {}

protected:
  void generateData() override;
};

} // namespace Trex

// GCOVR_EXCL_STOP
