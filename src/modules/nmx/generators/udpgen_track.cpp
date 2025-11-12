// Copyright (C) 2023 - 2025 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate artificial NMX readouts
//===----------------------------------------------------------------------===//

#include <modules/nmx/generators/TrackReadoutGenerator.h>
#include <memory> // Add include for std::unique_ptr
// GCOVR_EXCL_START

using namespace vmm3;
using VMM3Data = VMM3Parser::VMM3Data;

int main(int argc, char *argv[]) {

  Nmx::TrackReadoutGenerator NmxGen;
  uint8_t DataSize = sizeof(VMM3Data);
  NmxGen.setReadoutDataSize(DataSize);

  NmxGen.argParse(argc, argv);

  std::unique_ptr<FunctionGenerator> readoutTimeGenerator =
      std::make_unique<DistributionGenerator>(NmxGen.Settings.Frequency);
  NmxGen.initialize(std::move(readoutTimeGenerator));

  NmxGen.transmitLoop();

  return 0;
}
// GCOVR_EXCL_STOP
