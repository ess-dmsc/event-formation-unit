// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Main entry for freia
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <modules/vmm/VMMBase.h>

int main(int argc, char *argv[]) {
  MainProg Main("freia", argc, argv);

  auto Detector = new VMM::VMMBase(Main.DetectorSettings);

  return Main.run(Detector);
}
