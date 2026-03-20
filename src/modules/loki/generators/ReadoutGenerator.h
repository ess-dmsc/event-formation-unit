// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial CAEN readouts with variable number
/// of readouts
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <caen/readout/DataParser.h>
#include <common/testutils/DataFuzzer.h>
#include <generators/essudpgen/ReadoutGeneratorBase.h>

namespace caen {
class ReadoutGenerator : public ReadoutGeneratorBase {
public:
  ReadoutGenerator() : ReadoutGeneratorBase(DetectorType::LOKI) {}

protected:
  void generateData() override;
};

} // namespace caen
// GCOVR_EXCL_STOP
