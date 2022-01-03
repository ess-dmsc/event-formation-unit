// Copyright (C) 2019-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Jalousie detector base class
///
//===----------------------------------------------------------------------===//

#include <jalousie/JalousieBase.h>
#include <common/detector/Detector.h>

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
