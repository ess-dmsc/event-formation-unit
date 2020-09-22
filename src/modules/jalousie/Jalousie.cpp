/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Jalousie detector
///
//===----------------------------------------------------------------------===//

#include <jalousie/JalousieBase.h>
#include <common/Detector.h>

static struct Jalousie::CLISettings LocalJalousieSettings;

void SetCLIArguments(CLI::App& parser) {
  parser.add_option("-f,--file", LocalJalousieSettings.ConfigFile,
                    "Jalousie specific config file")->group("Jalousie")->
      required()->configurable(true);
}

PopulateCLIParser PopulateParser{SetCLIArguments};

class JALOUSIE : public Jalousie::JalousieBase {
public:
  explicit JALOUSIE(BaseSettings Settings)
      : Jalousie::JalousieBase(std::move(Settings), LocalJalousieSettings) {}
};

DetectorFactory<JALOUSIE> Factory;
