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

#include <cbm/CbmTypes.h>
#include <cbm/geometry/Parser.h>
#include <common/testutils/DataFuzzer.h>
#include <cstdint>
#include <generators/essudpgen/ReadoutGeneratorBase.h>
#include <generators/functiongenerators/FunctionGenerator.h>
#include <memory>
#include <optional>

namespace cbm {

enum class GeneratorType { Distribution, Linear, Fixed };

class ReadoutGenerator : public ReadoutGeneratorBase {

public:
  ReadoutGenerator();

private:
  // Beam monitors are always on logical fiber 22 (ring 11) and fen 0
  static constexpr uint8_t CBM_FIBER_ID = 22;
  static constexpr int MILLISEC = 1e3;

  std::unique_ptr<FunctionGenerator> Generator{nullptr};

  struct CbmGeneratorSettings {
    CbmType monitorType{CbmType::TTL};
    uint8_t FenId{0};
    uint8_t ChannelId{0};
    std::string generatorTypeStr{"Distribution"};
    GeneratorType generatorType{GeneratorType::Distribution};
    std::optional<uint32_t> genValue;
  } cbmSettings;

  void generateData() override;

  void generateIBMData(uint8_t *dataPtr);
  void generateTTLData(uint8_t *dataPtr);

  void distributionValueGenerator(Parser::CbmReadout *);
  void linearValueGenerator(Parser::CbmReadout *);
  void fixedValueGenerator(Parser::CbmReadout *);
};

} // namespace cbm

// GCOVR_EXCL_STOP
