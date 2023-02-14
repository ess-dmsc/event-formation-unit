// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Main entry for trex
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <modules/trex/TREXBase.h>

int main(int argc, char *argv[]) {
  MainProg Main("trex", argc, argv);

  auto Detector = new Trex::TrexBase(Main.DetectorSettings);

  return Main.run(Detector);
}
