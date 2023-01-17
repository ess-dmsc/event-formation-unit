// Copyright (C) 2022 - 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Main entry for vmm
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <modules/vmm/VMMBase.h>

int main(int argc, char *argv[]) {
  MainProg Main("vmm", argc, argv);

  auto Detector = new VMM::VMMBase(Main.DetectorSettings);

  return Main.run(Detector);
}
