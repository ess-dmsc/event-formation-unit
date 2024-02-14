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

#include <common/kafka/EV44Serializer.h>
#include <common/monitor/Histogram.h>
#include <common/readout/ess/ESSTime.h>
#include <common/readout/ess/Parser.h>
#include <ttlmonitor/Counters.h>
#include <ttlmonitor/TTLMonitorBase.h>
#include <ttlmonitor/geometry/Config.h>
#include <ttlmonitor/geometry/Parser.h>

namespace TTLMonitor {

class TTLMonitorInstrument {
public:
  /// \brief 'create' the TTLMonitor instrument
  /// based on settings the constructor loads both configuration
  /// and calibration data. It then initialises event builders and
  /// histograms
  TTLMonitorInstrument(Counters &counters, BaseSettings &settings);

  /// \brief process vmm-formatted monitor readouts
  void processMonitorReadouts(void);

  /// \brief dump readout data to HDF5
  // void dumpReadoutToFile(const ESSReadout::VMM3Parser::VMM3Data &Data);

public:
  /// \brief Stuff that 'ties' TTLMonitor together
  struct Counters &counters;
  BaseSettings &Settings;

  /// \brief
  Config Conf;

  /// \brief serialiser (and producer) for events
  std::vector<EV44Serializer *> SerializersPtr;

  /// \brief parser for the ESS Readout header
  ESSReadout::Parser ESSReadoutParser;

  /// \brief parser for TTLMon readout data
  Parser TTLMonParser;

  /// \brief for dumping raw VMM3 readouts to HDF5 files
  // std::shared_ptr<VMM3::ReadoutFile> DumpFile;
};

} // namespace TTLMonitor
