// Copyright (C) 2022 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial readout data for CBM beam monitor types
///        based on TTLMon ICD
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include "common/readout/ess/ESSTime.h"
#include <cbm/CbmTypes.h>
#include <common/testutils/DataFuzzer.h>
#include <cstdint>
#include <generators/essudpgen/ReadoutGeneratorBase.h>

namespace cbm {

class ReadoutGenerator : public ReadoutGeneratorBase {

public:
  ReadoutGenerator();

private:
  // Beam monitors are always on logical fiber 22 (ring 11) and fen 0
  static constexpr uint8_t CBM_FIBER_ID = 22;
  static constexpr uint8_t CBM_FEN_ID = 0;

  struct CbmGeneratorSettings {
    CbmType monitorType{CbmType::TTL};
  } cbmSettings;

  void generateData() override;
  ESSReadout::ESSTime::PulseTime generatePulseTime() override;

  void generateIBMData(uint8_t *dataPtr);
  void generateTTLData(uint8_t *dataPtr);
};
} // namespace cbm
// GCOVR_EXCL_STOP
