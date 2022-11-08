// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Main entry for miracles
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <modules/miracles/MiraclesBase.h>

int main(int argc, char * argv[]) {
  MainProg Main("miracles", argc, argv);

  auto Detector = new Miracles::MiraclesBase(Main.DetectorSettings);

  return Main.run(Detector);
}
