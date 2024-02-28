// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Main entry for Common Beam Monitor (CBM) detector module.
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <modules/cbm/TTLMonitorBase.h>

int main(int argc, char *argv[]) {
  MainProg Main("ttlmonitor", argc, argv);

  auto Detector = new TTLMonitor::TTLMonitorBase(Main.DetectorSettings);

  return Main.run(Detector);
}
