// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Main entry for loki
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <modules/caen/CaenBase.h>

int main(int argc, char *argv[]) {
  MainProg Main("loki", argc, argv);

  auto Detector = new Caen::CaenBase(Main.DetectorSettings, ESSReadout::Parser::LOKI);
  
  return Main.run(Detector);
}
