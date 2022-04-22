// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief TTLMonitor detector
///
//===----------------------------------------------------------------------===//

#include "TTLMonitorBase.h"
#include <common/detector/Detector.h>

static struct TTLMonitor::TTLMonitorSettings LocalTTLMonitorSettings;

// clang-format off
void SetCLIArguments(CLI::App __attribute__((unused)) & parser) {
  parser.add_option("-f, --file", LocalTTLMonitorSettings.ConfigFile,
           "TTLMonitor configuration (json) file")->group("TTLMonitor");

  parser.add_option("--dumptofile", LocalTTLMonitorSettings.FilePrefix,
           "dump to specified file")->group("TTLMonitor");

  parser.add_option("--reduce", LocalTTLMonitorSettings.ReduceEvents,
          "use 1 out of N readouts")->group("TTLMonitor");
}
// clang-format on

PopulateCLIParser PopulateParser{SetCLIArguments};

class TTLMON : public TTLMonitor::TTLMonitorBase {
public:
  explicit TTLMON(BaseSettings Settings)
      : TTLMonitor::TTLMonitorBase(std::move(Settings),
                                   LocalTTLMonitorSettings) {}
};

DetectorFactory<TTLMON> Factory;
