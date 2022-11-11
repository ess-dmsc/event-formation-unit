// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Main entry for perfgen
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <modules/perfgen/PerfGenBase.h>

int main(int argc, char *argv[]) {
  MainProg Main("perfgen", argc, argv);

  auto Detector = new PerfGen::PerfGenBase(Main.DetectorSettings);

  return Main.run(Detector);
}
