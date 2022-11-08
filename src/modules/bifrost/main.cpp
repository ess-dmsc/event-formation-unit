// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Main entry for Bifrost
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <modules/bifrost/BifrostBase.h>

int main(int argc, char * argv[]) {
  MainProg Main("bifrost", argc, argv);

  auto Detector = new Bifrost::BifrostBase(Main.DetectorSettings);

  return Main.run(Detector);
}
