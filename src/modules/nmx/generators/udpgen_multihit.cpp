// Copyright (C) 2023 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate artificial NMX readouts
//===----------------------------------------------------------------------===//

#include <modules/nmx/generators/MultiHitReadoutGenerator.h>
// GCOVR_EXCL_START

int main(int argc, char *argv[]) {

  Nmx::MultiHitReadoutGenerator NmxGen;
  uint8_t DataSize = sizeof(ESSReadout::VMM3Parser::VMM3Data);
  NmxGen.setReadoutDataSize(DataSize);

  NmxGen.argParse(argc, argv);

  std::shared_ptr<FunctionGenerator> distribution = DistributionGenerator::Factory(NmxGen.Settings.Frequency);
  NmxGen.main(distribution);

  NmxGen.transmitLoop();

  return 0;
}
// GCOVR_EXCL_STOP
