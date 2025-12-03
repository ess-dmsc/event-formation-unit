// Copyright (C) 2023 - 2025 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate artificial LET readouts
//===----------------------------------------------------------------------===//

#include <modules/trex/generators/LETReadoutGenerator.h>
// GCOVR_EXCL_START

using namespace vmm3;

int main(int argc, char *argv[]) {

  Trex::LETReadoutGenerator TrexGen;
  uint8_t TrexDataSize = sizeof(VMM3Parser::VMM3Data);
  TrexGen.setReadoutDataSize(TrexDataSize);

  TrexGen.argParse(argc, argv);

    std::unique_ptr<FunctionGenerator> readoutTimeGenerator = 
      std::make_unique<DistributionGenerator>(TrexGen.Settings.Frequency);
  TrexGen.initialize(std::move(readoutTimeGenerator));
  TrexGen.transmitLoop();

  return 0;
}
// GCOVR_EXCL_STOP
