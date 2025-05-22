// Copyright (C) 2023 - 2024 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate artificial DREAM readouts
//===----------------------------------------------------------------------===//

#include <modules/dream/generators/ReadoutGenerator.h>
// GCOVR_EXCL_START

int main(int argc, char *argv[]) {

  Dream::ReadoutGenerator DreamGen;
  uint8_t DreamDataSize = sizeof(Dream::DataParser::CDTReadout);
  DreamGen.setReadoutDataSize(DreamDataSize);

  DreamGen.argParse(argc, argv);

  std::shared_ptr<FunctionGenerator> distribution = DistributionGenerator::Factory(DreamGen.Settings.Frequency);
  DreamGen.initialize(distribution);
  DreamGen.transmitLoop();

  return 0;
}
// GCOVR_EXCL_STOP
