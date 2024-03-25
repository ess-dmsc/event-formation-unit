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

#include "cbm/CbmTypes.h"
#include <common/testutils/DataFuzzer.h>
#include <cstdint>
#include <generators/essudpgen/ReadoutGeneratorBase.h>

namespace cbm {

class ReadoutGenerator : public ReadoutGeneratorBase {

struct BmGeneratorSettings {
  uint8_t beamMonitortype{0};
} bmSettings;

public:
  ReadoutGenerator();

private:

struct CbmGeneratorSettings {
  CbmType monitorType{CbmType::TTL};
} cbmSettings;

  void generateData() override;
  const uint32_t TimeToFirstReadout{1000};
};
} // namespace cbm
// GCOVR_EXCL_STOP
