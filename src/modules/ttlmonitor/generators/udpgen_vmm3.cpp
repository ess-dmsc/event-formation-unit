// Copyright (C) 2023 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate artificial TTLMON readouts
//===----------------------------------------------------------------------===//

#include <modules/ttlmonitor/generators/ReadoutGeneratorVMM.h>
#include <modules/ttlmonitor/geometry/Parser.h>
// GCOVR_EXCL_START


int main(int argc, char *argv[]) {

  TTLMonitor::ReadoutGeneratorVMM TTLMonGen;
  uint8_t TTLMonDataSize = sizeof(TTLMonitor::Parser::Data);
  TTLMonGen.setReadoutDataSize(TTLMonDataSize);

  TTLMonGen.argParse(argc, argv);
  TTLMonGen.main();

  TTLMonGen.Settings.Type = ESSReadout::Parser::DetectorType::TTLMonitor;

  TTLMonGen.transmitLoop();

  return 0;
}
// GCOVR_EXCL_STOP
