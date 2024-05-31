// Copyright (C) 2022-2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Cbm is responsible for readout validation and
/// common beam monitor 'event formation'
/// Its functions are called from the main prcessing loop in CbmBase
//===----------------------------------------------------------------------===//

#pragma once

#include <cbm/CbmBase.h>
#include <cbm/Counters.h>
#include <cbm/geometry/Config.h>
#include <cbm/geometry/Parser.h>
#include <common/kafka/EV44Serializer.h>
#include <common/kafka/serializer/DA00HistogramSerializer.h>

namespace cbm {

class CbmInstrument {
public:
  /// \brief 'create' the CBM instrument
  /// based on settings the constructor loads both configuration
  /// and calibration data. It then initialises event builders and
  /// histograms
  CbmInstrument(Counters &counters, BaseSettings &settings,
                SerializerMap<EV44Serializer> &Ev44SerializerPtrs,
                SerializerMap<fbserializer::HistogramSerializer<int32_t>>
                    &HistogramSerializerPtrs);

  /// \brief process vmm-formatted monitor readouts
  void processMonitorReadouts(void);

  /// \brief dump readout data to HDF5
  // void dumpReadoutToFile(const ESSReadout::VMM3Parser::VMM3Data &Data);

public:
  /// \brief Stuff that 'ties' CBM together
  struct Counters &counters;
  BaseSettings &Settings;

  /// \brief
  Config Conf;

  /// \brief References for the serializers of the supported types
  SerializerMap<EV44Serializer> &Ev44SerializerMap;
  SerializerMap<fbserializer::HistogramSerializer<int32_t>>
      &HistogramSerializerMap;

  /// \brief parser for the ESS Readout header
  ESSReadout::Parser ESSReadoutParser;

  /// \brief parser for TTLMon readout data
  Parser CbmParser;

  /// \brief for dumping raw VMM3 readouts to HDF5 files
  // std::shared_ptr<VMM3::ReadoutFile> DumpFile;
};

} // namespace cbm
