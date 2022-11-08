// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Main entry for loki
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <modules/loki/LokiBase.h>

int main(int argc, char * argv[]) {
  MainProg Main("loki", argc, argv);

  auto Detector = new Loki::LokiBase(Main.DetectorSettings);

  return Main.run(Detector);
}
