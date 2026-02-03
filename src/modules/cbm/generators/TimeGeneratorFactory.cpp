// Copyright (C) 2025-2026 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Factory for creating CbmDataGenerator instances - Implementation
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <cbm/generators/TimeGeneratorFactory.h>
#include <generators/functiongenerators/DistributionGenerator.h>
#include <generators/functiongenerators/FixedValueGenerator.h>
#include <generators/functiongenerators/LinearGenerator.h>

namespace cbm {

std::unique_ptr<FunctionGenerator> TimeGeneratorFactory::createTimeGenerator(
    CbmType monitorType, uint16_t frequency, uint32_t numReadouts) {
  
  if (monitorType == CbmType::EVENT_0D || monitorType == CbmType::EVENT_2D) {
    return std::make_unique<DistributionGenerator>(frequency);
  } else if (monitorType == CbmType::IBM) {
    return std::make_unique<LinearGenerator>(frequency, numReadouts);
  } else {
    throw std::runtime_error("Unsupported monitor type");
  }
}

} // namespace cbm

// GCOVR_EXCL_STOP
