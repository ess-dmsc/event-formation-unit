// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief TTLMonitorInstrument is responsible for readout validation and
/// TTL monitor 'event formation'
/// Its functions are called from the main prcessing loop in TTLMonitorBase
//===----------------------------------------------------------------------===//

#pragma once

#include <common/kafka/EV42Serializer.h>
#include <common/monitor/Histogram.h>
#include <common/readout/ess/Parser.h>
#include <common/readout/ess/ESSTime.h>
#include <common/readout/vmm3/Readout.h>
#include <common/readout/vmm3/VMM3Parser.h>
#include <common/readout/vmm3/Hybrid.h>
#include <ttlmonitor/Counters.h>
#include <ttlmonitor/geometry/Config.h>
#include <ttlmonitor/TTLMonitorBase.h>

namespace TTLMonitor {

class TTLMonitorInstrument {
public:

  /// \brief 'create' the TTLMonitor instrument
  /// based on settings the constructor loads both configuration
  /// and calibration data. It then initialises event builders and
  /// histograms
  TTLMonitorInstrument(Counters & counters,
                  TTLMonitorSettings & moduleSettings,
                  EV42Serializer * serializer);

  /// \brief process vmm-formatted monitor readouts
  void processMonitorReadouts(void);

  /// \brief dump readout data to HDF5
  void dumpReadoutToFile(const ESSReadout::VMM3Parser::VMM3Data & Data);

  // \brief initialise the serializer. This is used both in TTLMonitorInstrument
  // and TTLMonitorBase. Called from TTLMonitorBase
  void setSerializer(EV42Serializer *serializer) { Serializer = serializer; }

public:
  /// \brief Stuff that 'ties' TTLMonitor together
  struct Counters & counters;
  TTLMonitorSettings & ModuleSettings;

  /// \brief
  Config Conf;

  /// \brief serialiser (and producer) for events
  EV42Serializer *Serializer{nullptr};

  /// \brief parser for the ESS Readout header
  ESSReadout::Parser ESSReadoutParser;

  /// \brief parser for VMM3 readout data
  ESSReadout::VMM3Parser VMMParser;

  /// \brief for dumping raw VMM3 readouts to HDF5 files
  std::shared_ptr<VMM3::ReadoutFile> DumpFile;
};

} // namespace