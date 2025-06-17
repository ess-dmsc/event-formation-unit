// Copyright (C) 2023 - 2025 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate artificial DREAM readouts
//===----------------------------------------------------------------------===//

#include "generators/functiongenerators/FunctionGenerator.h"
#include <memory>
#include <modules/dream/generators/ReadoutGenerator.h>
#include <utility>
// GCOVR_EXCL_START

int main(int argc, char *argv[]) {

  Dream::ReadoutGenerator DreamGen;
  uint8_t DreamDataSize = sizeof(Dream::DataParser::CDTReadout);
  DreamGen.setReadoutDataSize(DreamDataSize);

  DreamGen.argParse(argc, argv);

  std::unique_ptr<FunctionGenerator> readoutTimeGenerator =
      std::make_unique<DistributionGenerator>(DreamGen.Settings.Frequency);
  DreamGen.initialize(std::move(readoutTimeGenerator));
  DreamGen.transmitLoop();

  return 0;
}
// GCOVR_EXCL_STOP
