// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Main entry for cspec
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <modules/cspec/CSPECBase.h>

int main(int argc, char *argv[]) {
  MainProg Main("cspec", argc, argv);

  auto Detector = new Cspec::CspecBase(Main.DetectorSettings);

  return Main.run(Detector);
}
