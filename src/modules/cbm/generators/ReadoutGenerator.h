// Copyright (C) 2022 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial CBM readouts with variable number
/// of readouts
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <common/testutils/DataFuzzer.h>
#include <generators/essudpgen/ReadoutGeneratorBase.h>

namespace cbm {
class ReadoutGenerator : public ReadoutGeneratorBase {
public:
  using ReadoutGeneratorBase::ReadoutGeneratorBase;

protected:
  void generateData() override;
  const uint32_t TimeToFirstReadout{1000};
};
} // namespace cbm
// GCOVR_EXCL_STOP
