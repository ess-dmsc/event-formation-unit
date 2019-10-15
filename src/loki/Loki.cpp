/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Loki detector
///
//===----------------------------------------------------------------------===//

#include "LokiBase.h"
#include <common/Detector.h>

static struct Loki::LokiSettings LocalLokiSettings;

void SetCLIArguments(CLI::App __attribute__((unused)) & parser) {
}

PopulateCLIParser PopulateParser{SetCLIArguments};

class LOKI : public Loki::LokiBase {
public:
  explicit LOKI(BaseSettings Settings)
      : Loki::LokiBase(std::move(Settings), LocalLokiSettings) {}
};

DetectorFactory<LOKI> Factory;
