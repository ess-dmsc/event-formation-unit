// Copyright (C) 2017-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief MBCaen detector
///
//===----------------------------------------------------------------------===//

#include "MBCaenBase.h"
#include <common/detector/Detector.h>

static struct Multiblade::CAENSettings LocalMBCAENSettings;

void SetCLIArguments(CLI::App __attribute__((unused)) & parser) {
  parser.add_option("--dumptofile", LocalMBCAENSettings.FilePrefix,
                    "dump to specified file")->group("MBCAEN");

  parser.add_option("-f, --file", LocalMBCAENSettings.ConfigFile,
                    "Multi-Blade specific calibration (json) file")
                    ->group("MBCAEN");

  parser.add_option("--h5filesplit", LocalMBCAENSettings.H5SplitTime,
                    "Specify interval to split HDF5 files")
                    ->group("MBCAEN");
}

PopulateCLIParser PopulateParser{SetCLIArguments};

class MBCAEN : public Multiblade::CAENBase {
public:
  explicit MBCAEN(BaseSettings Settings)
      : Multiblade::CAENBase(std::move(Settings), LocalMBCAENSettings) {}
};

DetectorFactory<MBCAEN> Factory;
