// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Main entry for dream
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <modules/dream/DreamBase.h>

int main(int argc, char * argv[]) {
  MainProg Main("dream", argc, argv);

  auto Detector = new Dream::DreamBase(Main.DetectorSettings);

  return Main.run(Detector);
}
