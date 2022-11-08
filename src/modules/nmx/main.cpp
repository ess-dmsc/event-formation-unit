// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Main entry for cspec
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <modules/nmx/NMXBase.h>

int main(int argc, char * argv[]) {
  MainProg Main("nmx", argc, argv);

  auto Detector = new Nmx::NmxBase(Main.DetectorSettings);

  return Main.run(Detector);
}
