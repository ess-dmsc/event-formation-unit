// Copyright (C) 2023 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate artificial LOKI readouts
//===----------------------------------------------------------------------===//

#include <modules/loki/generators/ReadoutGenerator.h>
// GCOVR_EXCL_START

int main(int argc, char *argv[]) {

  Caen::ReadoutGenerator LokiGen;
  uint8_t LokiDataSize = sizeof(Caen::DataParser::CaenReadout);
  LokiGen.setReadoutDataSize(LokiDataSize);

  LokiGen.argParse(argc, argv);

  std::shared_ptr<DistributionGenerator> distribution = DistributionGenerator::Factory(LokiGen.Settings.Frequency);
  LokiGen.main(distribution);

  LokiGen.transmitLoop();

  return 0;
}
// GCOVR_EXCL_STOP
