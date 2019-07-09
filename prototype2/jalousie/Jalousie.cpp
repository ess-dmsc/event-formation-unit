/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Jalousie detector
///
//===----------------------------------------------------------------------===//

#include "JalousieBase.h"
#include <common/Detector.h>

void SetCLIArguments(CLI::App __attribute__((unused)) & parser) {
}

PopulateCLIParser PopulateParser{SetCLIArguments};

class JALOUSIE : public Jalousie::JalousieBase {
public:
  explicit JALOUSIE(BaseSettings Settings)
      : Jalousie::JalousieBase(std::move(Settings)) {}
};

DetectorFactory<JALOUSIE> Factory;
